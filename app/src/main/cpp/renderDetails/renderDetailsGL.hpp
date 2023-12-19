/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
 *
 *  This file is part of AmazingLabyrinth.
 *
 *  AmazingLabyrinth is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AmazingLabyrinth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AmazingLabyrinth.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_GL_HPP
#include <memory>
#include <string>

#include <GLES3/gl3.h>
#include <boost/variant.hpp>

#include "../common.hpp"
#include "renderDetails.hpp"
#include "../graphicsGL.hpp"
#include "../levelDrawer/drawObjectTable/drawObjectTableGL.hpp"
#include "../levelDrawer/drawObjectTable/drawObjectTable.hpp"
#include "../levelDrawer/modelTable/modelTableGL.hpp"

class RenderLoaderGL;
namespace renderDetails {
    struct ParametersObjectWithShadowsGL : public ParametersPerspective {
        std::shared_ptr<graphicsGL::Framebuffer> shadowsFB;

        ParametersObjectWithShadowsGL(
                ParametersPerspective const &parameters, std::shared_ptr<graphicsGL::Framebuffer> fb)
                : ParametersPerspective(parameters), shadowsFB{std::move(fb)} {}

        ~ParametersObjectWithShadowsGL() override = default;
    };

    struct ParametersDarkObjectGL : public ParametersPerspective {
        std::array<std::shared_ptr<graphicsGL::Framebuffer>, renderDetails::numberOfShadowMapsDarkMaze> darkFramebuffers;

        ParametersDarkObjectGL(ParametersPerspective const &parameters,
                               std::array<std::shared_ptr<graphicsGL::Framebuffer>, renderDetails::numberOfShadowMapsDarkMaze> fbs)
                : ParametersPerspective(parameters), darkFramebuffers(std::move(fbs)) {}

        ~ParametersDarkObjectGL() override = default;
    };

    class DrawObjectDataGL : public DrawObjectData {
    public:
        // no work is needed for this function in GL.  Just return true to indicate that it
        // succeeded
        virtual bool updateTextureData(
                std::shared_ptr<CommonObjectData> const &,
                std::shared_ptr<levelDrawer::TextureDataGL> const &)
        {
            return true;
        }

        ~DrawObjectDataGL() override = default;
    };

    class DoNothingVisitor : public boost::static_visitor<std::vector<float>> {
    public:
        template <typename value_type>
        std::vector<float> operator()(std::vector<value_type> const &input) const
        {
            std::vector<float> results(input.size());
            std::copy(input.begin(), input.end(), results.begin());
            return std::move(results);
        }
    };

    class Shader {
    public:
        inline GLuint shaderID() const { return m_shaderID; }

        Shader(Shader const &) = delete;

        Shader(Shader &&other)
                : m_shaderID{other.m_shaderID}
        {
            other.m_shaderID = 0;
        }

        Shader &operator=(Shader const &) = delete;

        Shader &operator=(Shader &&other) {
            if (this != &other) {
                glDeleteShader(m_shaderID);
                m_shaderID = 0;
                std::swap(m_shaderID, other.m_shaderID);
            }

            return *this;
        }

        Shader(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::string const &shaderFile,
                GLenum shaderType)
                : m_shaderID{0}
        {
            std::vector<char> shader = readFile(gameRequester, shaderFile);

            // Create the shaders
            m_shaderID = glCreateShader(shaderType);

            // Compile the Shader
            char const *sourcePointer = shader.data();
            GLint shaderLength = shader.size();
            glShaderSource(m_shaderID, 1, &sourcePointer, &shaderLength);
            glCompileShader(m_shaderID);

            // Check the Shader
            GLint Result = GL_TRUE;
            glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &Result);
            if (Result == GL_FALSE) {
                GLint InfoLogLength = 0;
                glGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
                if (InfoLogLength > 0) {
                    std::vector<char> shaderErrorMessage(InfoLogLength + 1);
                    glGetShaderInfoLog(m_shaderID, InfoLogLength, &InfoLogLength,
                                       shaderErrorMessage.data());
                    glDeleteShader(m_shaderID);
                    if (shaderErrorMessage[0] == '\0') {
                        throw std::runtime_error("shader: " + shaderFile + " compile error.");
                    } else {
                        throw std::runtime_error(std::string("shader: ") + shaderFile + " compile error: " +
                                                 shaderErrorMessage.data());
                    }
                } else {
                    throw std::runtime_error("shader: " + shaderFile + " compile error.");
                }
            }
        }

        ~Shader() {
            glDeleteShader(m_shaderID);
        }
    private:
        GLuint m_shaderID;
    };

    class GLProgram {
    public:
        GLuint programID() { return m_programID; }

        GLProgram(GLProgram const &) = delete;

        GLProgram(GLProgram &&other)
        : m_programID{other.m_programID}
        {
            other.m_programID = 0;
        }

        GLProgram &operator=(GLProgram const &) = delete;

        GLProgram &operator=(GLProgram &&other) {
            if (this != &other) {
                glDeleteProgram(m_programID);
                m_programID = 0;
                std::swap(m_programID, other.m_programID);
            }

            return *this;
        }

        GLProgram(
                std::vector<std::shared_ptr<Shader>> const &shaders)
                : m_programID{0}
        {
            if (shaders.empty()) {
                throw std::runtime_error("A shader was incorrectly initialized when loading the GL program.");
            }

            // Create the program
            m_programID = glCreateProgram();

            for (auto const &shader : shaders) {
                glAttachShader(m_programID, shader->shaderID());
            }

            glLinkProgram(m_programID);

            // glLinkProgram doc pages state that once the link step is done, programs can be
            // detached, deleted, etc.
            for (auto const &shader : shaders) {
                glDetachShader(m_programID, shader->shaderID());
            }

            // Check the program
            GLint Result = GL_TRUE;
            glGetProgramiv(m_programID, GL_LINK_STATUS, &Result);

            if (Result == GL_FALSE) {
                GLint InfoLogLength = 0;
                glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
                if (InfoLogLength > 0) {
                    std::vector<char> ProgramErrorMessage(InfoLogLength + 1, 0);
                    glGetProgramInfoLog(m_programID, InfoLogLength, nullptr, ProgramErrorMessage.data());
                    glDeleteProgram(m_programID);
                    throw std::runtime_error(ProgramErrorMessage.data());
                } else {
                    glDeleteProgram(m_programID);
                    throw std::runtime_error("glLinkProgram error.");
                }
            }
        }

        ~GLProgram() {
            glDeleteProgram(m_programID);
        }

    private:
        GLuint m_programID;
    };

    class RenderDetailsGL : public RenderDetails {
    public:
        // For postprocessing results written to an image buffer whose contents are put in input.
        // This function is for render details that produce results to be used by the CPU.  Most
        // render details don't need this.  Provide this default function which copies the
        // source into the destination (probably will never be called).
        virtual void postProcessImageBuffer(
                std::shared_ptr<renderDetails::CommonObjectData> const &,
                PostprocessingDataInputGL const &input,
                std::vector<float> &results)
        {
            DoNothingVisitor doNothing;
            results = boost::apply_visitor(doNothing, input);
        }

        // GL commands that need to occur before the main draw.  They can be things like generating
        // a shadow map or other draws to a framebuffer.  Most shaders don't need these commands so
        // the default does nothing.
        virtual void preMainDraw(
                uint32_t,
                levelDrawer::CommonObjectDataList const &,
                levelDrawer::DrawObjectTableGList const &,
                std::set<levelDrawer::ZValueReference> const & /* starter */,
                std::set<levelDrawer::ZValueReference> const & /* level */,
                std::set<levelDrawer::ZValueReference> const & /* finisher */)
        {}

        virtual bool structuralChangeNeeded(
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails)
        {
            return surfaceDetails->useIntTexture != m_usesIntSurface ||
                surfaceDetails->surfaceWidth != m_surfaceWidth ||
                surfaceDetails->surfaceHeight != m_surfaceHeight;
        }

        virtual void reload(std::shared_ptr<GameRequester> const &,
                            std::shared_ptr<RenderLoaderGL> const &,
                            std::shared_ptr<graphicsGL::SurfaceDetails> const &)
        {
            // do nothing.  For GL, we need to dump all render details and reload everything.
        }

        virtual void draw(
                uint32_t modelMatrixID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
                std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
                std::set<levelDrawer::ZValueReference>::iterator endZValRefs) = 0;

        virtual bool overrideClearColor(glm::vec4 &) {
            return false;
        }

        RenderDetailsGL(
            uint32_t inWidth,
            uint32_t inHeight,
            bool usesIntSurface)
            : RenderDetails{inWidth, inHeight},
            m_usesIntSurface{usesIntSurface}
        {}

        ~RenderDetailsGL() override = default;

    protected:
        static void drawVertices(
                GLuint programID,
                std::shared_ptr<levelDrawer::ModelDataGL> const &modelData,
                bool useVertexNormals = false);

        bool m_usesIntSurface;
    };

    using ReferenceGL = Reference<RenderDetailsGL, levelDrawer::TextureDataGL, DrawObjectDataGL>;
}

#endif // AMAZING_LABYRINTH_RENDER_DETAILS_GL_HPP