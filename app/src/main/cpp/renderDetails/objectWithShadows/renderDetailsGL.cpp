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
#include <memory>
#include <vector>

#include "../renderDetailsGL.hpp"
#include "renderDetailsGL.hpp"
#include "../shadows/renderDetailsGL.hpp"
#include "../../graphicsGL.hpp"

namespace objectWithShadows {
    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        auto rd = std::make_shared<RenderDetailsGL>(gameRequester, parameters.width,
                                                    parameters.height);

        renderDetails::ParametersWithShadowsGL const &p =
                dynamic_cast<renderDetails::ParametersWithShadowsGL const &>(parameters);

        auto cod = std::make_shared<CommonObjectDataGL>(p.shadowsFB,
                parameters.width / static_cast<float>(parameters.height), config);

        return createReference(std::move(rd), std::move(cod));
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        renderDetails::ParametersWithShadowsGL const &p =
                dynamic_cast<renderDetails::ParametersWithShadowsGL const &>(parameters);

        auto cod = std::make_shared<CommonObjectDataGL>(p.shadowsFB,
                parameters.width / static_cast<float>(parameters.height), config);

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
                    std::shared_ptr<levelDrawer::TextureData>,
                    glm::mat4 modelMatrix) -> std::shared_ptr<renderDetails::DrawObjectDataGL>
            {
                return std::make_shared<DrawObjectDataGL>(std::move(modelMatrix));
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
            std::vector<size_t> const &drawObjectsIndices)
    {
        if (drawObjectsIndices.empty() ||
            drawObjTable == nullptr ||
            commonObjectData == nullptr) {
            return;
        }

        glCullFace(GL_BACK);
        checkGraphicsError();

        drawLevelType(m_textureProgramID, true, modelMatrixID,
            commonObjectData, drawObjTable, drawObjectsIndices);

        drawLevelType(m_colorProgramID, false, modelMatrixID,
            commonObjectData, drawObjTable, drawObjectsIndices);
    }

    void RenderDetailsGL::drawLevelType(
            GLuint programID,
            bool drawObjsWithTexture,
            uint32_t modelMatrixID,
            std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
            std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
            std::vector<size_t> const &drawObjectsIndices)
    {
        glUseProgram(programID);
        checkGraphicsError();

        GLint MatrixID;
        auto projView = commonObjectData->getProjViewForLevel();

        // the projection matrix
        MatrixID = glGetUniformLocation(programID, "proj");
        checkGraphicsError();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(projView.first)[0][0]);
        checkGraphicsError();

        // the view matrix
        MatrixID = glGetUniformLocation(programID, "view");
        checkGraphicsError();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(projView.second)[0][0]);
        checkGraphicsError();

        // the view matrix from the light source point of view
        auto lightSpaceMatrix = commonObjectData->getViewLightSource();
        MatrixID = glGetUniformLocation(programID, "lightSpaceMatrix");
        checkGraphicsError();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
        checkGraphicsError();

        GLint lightPosID = glGetUniformLocation(programID, "lightPos");
        checkGraphicsError();
        glm::vec3 lightPos = commonObjectData->getLightSource();
        glUniform3fv(lightPosID, 1, &lightPos[0]);
        checkGraphicsError();

        auto cod = dynamic_cast<CommonObjectDataGL*>(commonObjectData.get());
        if (!cod) {
            throw std::runtime_error("Invalid common object data type");
        }
        auto fb = cod->m_shadowsFramebuffer;

        GLint textureID = glGetUniformLocation(programID, "texShadowMap");
        checkGraphicsError();
        glActiveTexture(GL_TEXTURE0);
        checkGraphicsError();
        glBindTexture(GL_TEXTURE_2D, fb->depthImage());
        checkGraphicsError();
        glUniform1i(textureID, 0);
        checkGraphicsError();

        MatrixID = glGetUniformLocation(programID, "model");
        checkGraphicsError();

        GLint normalMatrixID = glGetUniformLocation(programID, "normalMatrix");
        checkGraphicsError();

        if (drawObjsWithTexture) {
            textureID = glGetUniformLocation(programID, "texSampler");
            checkGraphicsError();
        }

        for (auto drawObjIndex : drawObjectsIndices) {
            auto drawObj = drawObjTable->drawObject(drawObjIndex);
            auto modelData = drawObj->modelData();
            auto textureData = drawObj->textureData();

            if ((textureData == nullptr && drawObjsWithTexture) ||
                    (textureData != nullptr && !drawObjsWithTexture))
            {
                continue;
            }

            if (drawObjsWithTexture) {
                glActiveTexture(GL_TEXTURE1);
                checkGraphicsError();
                glBindTexture(GL_TEXTURE_2D, textureData->handle());
                checkGraphicsError();
                glUniform1i(textureID, 1);
                checkGraphicsError();
            }

            size_t nbrObjData = drawObj->numberObjectsData();
            for (size_t i = 0; i < nbrObjData; i++) {
                auto objData = drawObj->objData(i);
                auto modelMatrix = objData->modelMatrix(modelMatrixID);

                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
                checkGraphicsError();

                glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
                glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, &normalMatrix[0][0]);
                checkGraphicsError();

                drawVertices(programID, modelData);
            }
        }

    }

    RenderDetailsGL::RenderDetailsGL(
            std::shared_ptr<GameRequester> const &inGameRequester,
            uint32_t inWidth, uint32_t inHeight)
        : renderDetails::RenderDetailsGL(inWidth, inHeight),
        m_textureProgramID{loadShaders(inGameRequester, SHADER_VERT_FILE, TEXTURE_SHADER_FRAG_FILE)},
        m_colorProgramID{loadShaders(inGameRequester, SHADER_VERT_FILE, COLOR_SHADER_FRAG_FILE)}
    {}
}
