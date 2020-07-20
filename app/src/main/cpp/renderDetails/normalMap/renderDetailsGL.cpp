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
#include "config.hpp"

namespace normalMap {
    RenderDetailsGL::RenderDetailsGL(std::shared_ptr<GameRequester> const &inGameRequester,
                                             uint32_t inWidth, uint32_t inHeight)
            : renderDetails::RenderDetailsGL(inWidth, inHeight),
              m_depthProgramID{loadShaders(inGameRequester, LINEAR_DEPTH_VERT_FILE, SIMPLE_FRAG_FILE)}
    {}

    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        auto rd = std::make_shared<RenderDetailsGL>(gameRequester, parameters.width,
                                                    parameters.height);

        auto parametersWithExtra =
                static_cast<renderDetails::ParametersWithWidthHeightAtDepthGL const &>(parameters);
        auto cod = std::make_shared<CommonObjectDataGL>(
                parametersWithExtra.nearestDepth,
                parametersWithExtra.farthestDepth,
                config,
                parametersWithExtra.width,
                parametersWithExtra.height);

        return createReference(rd, cod);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            renderDetails::ParametersGL const &parameters,
            Config const &config)
    {
        auto parametersWithExtra =
                static_cast<renderDetails::ParametersWithWidthHeightAtDepthGL const &>(parameters);

        auto cod = std::make_shared<CommonObjectDataGL>(
                parametersWithExtra.nearestDepth,
                parametersWithExtra.farthestDepth,
                config,
                parametersWithExtra.width,
                parametersWithExtra.height);

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
        glCullFace(GL_BACK);
        checkGraphicsError();

        if (drawObjectsIndices.empty() ||
            drawObjTable == nullptr ||
            commonObjectData == nullptr)
        {
            return;
        }

        auto projView = commonObjectData->getProjViewForLevel();

        auto cod = dynamic_cast<CommonObjectDataGL*>(commonObjectData.get());

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

        GLint loc = glGetUniformLocation(m_depthProgramID, "nearestDepth");
        glUniform1f(loc, cod->nearestDepth());
        checkGraphicsError();

        loc = glGetUniformLocation(m_depthProgramID, "farthestDepth");
        glUniform1f(loc, cod->farthestDepth());
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
} // namespace shadows