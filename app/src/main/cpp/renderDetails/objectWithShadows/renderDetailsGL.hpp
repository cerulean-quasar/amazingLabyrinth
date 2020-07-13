/**
 * Copyright 2020 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_GL_HPP

#include <memory>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "config.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"
#include "../../graphicsGL.hpp"
#include "../../renderLoader/renderLoaderGL.hpp"

namespace objectWithShadows {
    class RenderDetailsGL;
    class CommonObjectDataGL : public renderDetails::CommonObjectDataPerspective {
        friend RenderDetailsGL;
    public:
        glm::mat4 getPerspectiveMatrixForLevel() override {
            /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
             * view planes.
             */
            return getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane,
                                        false, false);
        }
    private:
        std::shared_ptr<graphicsGL::Framebuffer> m_shadowsFramebuffer;

        CommonObjectDataGL(
                std::shared_ptr<graphicsGL::Framebuffer> shadowsFramebuffer,
                float aspectRatio,
                Config const &config)
            : CommonObjectDataPerspective(config.viewAngle, aspectRatio, config.nearPlane, config.farPlane,
                    config.viewPoint, config.lookAt, config.up),
            m_shadowsFramebuffer(std::move(shadowsFramebuffer))
        {}
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectDataGL {
        friend RenderDetailsGL;
    public:
        void update(glm::mat4 const &modelMatrix) override {
            m_modelMatrix = modelMatrix;
        }

        ~DrawObjectDataGL() override = default;
    private:
        DrawObjectDataGL(
                glm::mat4 inModelMatrix)
                : renderDetails::DrawObjectDataGL{},
                  m_modelMatrix{std::move(inModelMatrix)}
        {}

        glm::mat4 m_modelMatrix;
    };

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        static char const *name() { return m_name; }
        static renderDetails::ReferenceGL loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                renderDetails::ParametersGL const &parameters,
                Config const &config);

        static renderDetails::ReferenceGL loadExisting(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
                renderDetails::ParametersGL const &parameters,
                Config const &config);

        ~RenderDetailsGL() override = default;

    private:
        static char constexpr const *m_name = "objectWithShadows";
        static char constexpr const *SHADER_VERT_FILE = "shaders/shaderGL.vert";
        static char constexpr const *TEXTURE_SHADER_FRAG_FILE = "shaders/shaderGL.frag";
        static char constexpr const *COLOR_SHADER_FRAG_FILE = "shaders/shaderGL.frag";

        GLuint m_textureProgramID;
        GLuint m_colorProgramID;

        static renderDetails::ReferenceGL createReference(
                std::shared_ptr<renderDetails::RenderDetailsGL> rd,
                std::shared_ptr<CommonObjectDataGL> cod);

        RenderDetailsGL(std::shared_ptr<GameRequester> const &inGameRequester,
                        uint32_t inWidth, uint32_t inHeight);
    };
}

#endif // AMAZING_LABYRINTH_TEXTUREWITHSHADOWS_RENDER_DETAILS_GL_HPP