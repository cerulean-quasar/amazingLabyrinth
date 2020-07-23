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

#include "../renderDetailsGL.hpp"
#include "renderDetailsGL.hpp"

namespace shadows {
    RenderDetailsGL::RenderDetailsGL(std::shared_ptr<GameRequester> const &inGameRequester,
                                             uint32_t inWidth, uint32_t inHeight)
            : renderDetails::RenderDetailsGL(inWidth, inHeight),
              m_depthProgramID{loadShaders(inGameRequester, DEPTH_VERT_FILE, SIMPLE_FRAG_FILE)}
    {}

    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<renderDetails::Parameters> const &parameters,
            Config const &config)
    {
        auto rd = std::make_shared<RenderDetailsGL>(gameRequester, parameters->width,
                                                    parameters->height);

        auto cod = std::make_shared<CommonObjectDataGL>(
                parameters->width / static_cast<float>(parameters->height), config);

        return createReference(rd, cod);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            std::shared_ptr<renderDetails::Parameters> const &parameters,
            Config const &config)
    {
        auto cod = std::make_shared<CommonObjectDataGL>(
                parameters->width / static_cast<float>(parameters->height), config);

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
        // set the shader to use
        glUseProgram(m_depthProgramID);
        checkGraphicsError();
        glCullFace(GL_FRONT);
        checkGraphicsError();

        if (drawObjectsIndices.empty() ||
            drawObjTable == nullptr ||
            commonObjectData == nullptr)
        {
            return;
        }

        auto projView = commonObjectData->getProjViewForLevel();

        GLint MatrixID;

        // the projection matrix
        MatrixID = glGetUniformLocation(m_depthProgramID, "proj");
        checkGraphicsError();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(projView.first)[0][0]);
        checkGraphicsError();

        // the view matrix
        MatrixID = glGetUniformLocation(m_depthProgramID, "view");
        checkGraphicsError();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(projView.second)[0][0]);
        checkGraphicsError();

        MatrixID = glGetUniformLocation(m_depthProgramID, "model");
        checkGraphicsError();

        for (auto drawObjIndex : drawObjectsIndices) {
            auto drawObj = drawObjTable->drawObject(drawObjIndex);
            auto modelData = drawObj->modelData();

            size_t nbrObjData = drawObj->numberObjectsData();
            for (size_t i = 0; i < nbrObjData; i++) {
                auto objData = drawObj->objData(i);
                auto modelMatrix = objData->modelMatrix(modelMatrixID);

                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
                checkGraphicsError();

                drawVertices(m_depthProgramID, modelData);
            }
        }
    }

    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL, Config, renderDetails::ParametersGL> registerGL();

} // namespace shadows