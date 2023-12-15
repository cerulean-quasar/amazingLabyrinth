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
#include <memory>
#include <vector>

#include "../renderDetailsGL.hpp"
#include "renderDetailsGL.hpp"
#include "../shadows/renderDetailsGL.hpp"
#include "../../graphicsGL.hpp"

namespace objectNoShadows {

    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            char const *name,
            std::vector<char const *> const &shaders,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto parameters =
                dynamic_cast<renderDetails::ParametersPerspective*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        if (shaders.size() != 3) {
            throw std::runtime_error("Wrong number of shaders for Normal Map Render Details.");
        }

        auto rd = std::make_shared<RenderDetailsGL>(
                name, shaders[0], shaders[1], shaders[2],
                gameRequester, surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight,
                surfaceDetails->useIntTexture);

        auto cod = std::make_shared<CommonObjectDataGL>(*parameters,
                surfaceDetails->surfaceWidth / static_cast<float>(surfaceDetails->surfaceHeight));

        return createReference(std::move(rd), std::move(cod));
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto rd = dynamic_cast<RenderDetailsGL*>(rdBase.get());
        if (rd == nullptr) {
            throw std::runtime_error("Invalid render details type.");
        }

        if (rd->m_surfaceWidth != surfaceDetails->surfaceWidth ||
            rd->m_surfaceHeight != surfaceDetails->surfaceHeight)
        {
            rd->m_surfaceWidth = surfaceDetails->surfaceWidth;
            rd->m_surfaceHeight = surfaceDetails->surfaceHeight;
        }

        auto parameters =
                dynamic_cast<renderDetails::ParametersPerspective*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        auto cod = std::make_shared<CommonObjectDataGL>(*parameters,
                surfaceDetails->surfaceWidth / static_cast<float>(surfaceDetails->surfaceHeight));

        return createReference(std::move(rdBase), std::move(cod));
    }

    renderDetails::ReferenceGL RenderDetailsGL::createReference(
            std::shared_ptr<renderDetails::RenderDetailsGL> rd,
            std::shared_ptr<CommonObjectDataGL> cod)
    {
        renderDetails::ReferenceGL ref;
        ref.createDrawObjectData = renderDetails::ReferenceGL::CreateDrawObjectData(
            [] (
                    std::shared_ptr<renderDetails::DrawObjectDataGL> const &,
                    std::shared_ptr<levelDrawer::TextureData> const &,
                    glm::mat4 const &modelMatrix) -> std::shared_ptr<renderDetails::DrawObjectDataGL>
            {
                return std::make_shared<DrawObjectDataGL>(modelMatrix);
            });

        ref.getProjViewForLevel = renderDetails::ReferenceGL::GetProjViewForLevel(
                [cod] () -> std::pair<glm::mat4, glm::mat4> {
                    return cod->getProjViewForLevel();
                });

        ref.renderDetails = std::move(rd);
        ref.commonObjectData = std::move(cod);
        return std::move(ref);
    }

    void RenderDetailsGL::draw(
            uint32_t modelMatrixID,
            std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
            std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs)
    {
        glCullFace(GL_BACK);
        checkGraphicsError();

        auto cod = dynamic_cast<CommonObjectDataGL *>(commonObjectData.get());
        if (!cod) {
            throw std::runtime_error("Invalid common object data type");
        }

        GLuint textureProgramID = m_textureProgram->programID();
        GLuint colorProgramID = m_colorProgram->programID();
        GLuint programID = textureProgramID;
        GLint MatrixID = -1;
        GLint normalMatrixID = -1;
        GLint textureID = -1;

        for (auto it = beginZValRefs; it != endZValRefs; it++) {
            auto &drawObj = drawObjTable->drawObject(it->drawObjectReference);
            auto &modelData = drawObj->modelData();
            auto &textureData = drawObj->textureData();
            if (it == beginZValRefs ||
                (programID == textureProgramID && !textureData) ||
                    (programID == colorProgramID && textureData)) {
                programID = textureData ? textureProgramID : colorProgramID;
                glUseProgram(programID);
                checkGraphicsError();

                auto projView = cod->getProjViewForLevel();

                // the projection matrix * the view matrix
                MatrixID = glGetUniformLocation(programID, "projView");
                checkGraphicsError();
                glm::mat4 projTimesView = projView.first * projView.second;
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projTimesView[0][0]);
                checkGraphicsError();

                GLint lightPosID = glGetUniformLocation(programID, "lightPos");
                checkGraphicsError();
                glm::vec3 lightPos = cod->getLightSource(0);
                glUniform3fv(lightPosID, 1, &lightPos[0]);
                checkGraphicsError();

                MatrixID = glGetUniformLocation(programID, "model");
                checkGraphicsError();

                normalMatrixID = glGetUniformLocation(programID, "normalMatrix");
                checkGraphicsError();

                if (textureData) {
                    textureID = glGetUniformLocation(programID, "texSampler");
                    checkGraphicsError();
                }
            }

            if (textureData) {
                glActiveTexture(GL_TEXTURE1);
                checkGraphicsError();
                glBindTexture(GL_TEXTURE_2D, textureData->handle());
                checkGraphicsError();
                glUniform1i(textureID, 1);
                checkGraphicsError();
            }

            auto objData = drawObj->objData(it->drawObjectDataReference.get());
            auto modelMatrix = objData->modelMatrix(modelMatrixID);

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
            checkGraphicsError();

            glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
            glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, &normalMatrix[0][0]);
            checkGraphicsError();

            drawVertices(programID, modelData);
        }
    }

    RenderDetailsGL::RenderDetailsGL(
            char const *name,
            char const *vertexShaderFile,
            char const *textureFragShaderFile,
            char const *colorFragShaderFile,
            std::shared_ptr<GameRequester> const &inGameRequester,
            uint32_t inWidth, uint32_t inHeight, bool usesIntSurface)
        : renderDetails::RenderDetailsGL(inWidth, inHeight, usesIntSurface),
        m_renderDetailsName{name},
        m_textureProgram{},
        m_colorProgram{}
    {
        auto vertexShader = cacheShader(inGameRequester, vertexShaderFile, GL_VERTEX_SHADER);
        auto textureFragShader = cacheShader(inGameRequester, textureFragShaderFile, GL_FRAGMENT_SHADER);
        auto colorFragShader = cacheShader(inGameRequester, colorFragShaderFile, GL_FRAGMENT_SHADER);

        m_textureProgram = std::make_shared<renderDetails::GLProgram>(
                std::vector{vertexShader, textureFragShader});
        m_colorProgram = std::make_shared<renderDetails::GLProgram>(
                std::vector{vertexShader, colorFragShader});
    }

    char constexpr const *SHADER_VERT_GL_FILE = "shaders/shaderNoShadowsGL.vert";
    char constexpr const *TEXTURE_SHADER_FRAG_GL_FILE = "shaders/shaderNoShadowsGL.frag";
    char constexpr const *COLOR_SHADER_FRAG_GL_FILE = "shaders/colorNoShadowsGL.frag";
    char constexpr const *TEXTURE_DARK_V2_FRAG_GL_FILE = "shaders/darkV2TextureGL.frag";
    char constexpr const *COLOR_DARK_V2_FRAG_GL_FILE = "shaders/darkV2ColorGL.frag";
    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL> registerGL(
            objectNoShadowsRenderDetailsName,
            std::vector<char const *>{SHADER_VERT_GL_FILE, TEXTURE_SHADER_FRAG_GL_FILE, COLOR_SHADER_FRAG_GL_FILE});
    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL> registerDarkV2ObjectGL(
            darkV2ObjectRenderDetailsName,
            std::vector<char const *>{SHADER_VERT_GL_FILE, TEXTURE_DARK_V2_FRAG_GL_FILE, COLOR_DARK_V2_FRAG_GL_FILE});
} // objectNoWithShadows
