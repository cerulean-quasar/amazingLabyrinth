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
    float constexpr const Config::nearPlane;
    float constexpr const Config::farPlane;
    glm::vec3 constexpr const Config::viewPoint;
    glm::vec3 constexpr const Config::lookAt;
    glm::vec3 constexpr const Config::up;

    RenderDetailsGL::RenderDetailsGL(std::shared_ptr<GameRequester> const &inGameRequester,
                                             uint32_t inWidth, uint32_t inHeight,
                                             bool isIntSurface)
            : renderDetails::RenderDetailsGL(inWidth, inHeight, isIntSurface),
              m_programID{0}
    {
        loadPipeline(inGameRequester);
    }

    void RenderDetailsGL::loadPipeline(std::shared_ptr<GameRequester> const &inGameRequester) {
        if (m_programID != 0) {
            glDeleteProgram(m_programID);
            checkGraphicsError();
        }

        if (m_usesIntSurface) {
            m_programID = loadShaders(inGameRequester, NORMAL3_VERT_FILE,
                                           SIMPLE3_FRAG_FILE);
        } else {
            m_programID = loadShaders(inGameRequester, NORMAL_VERT_FILE,
                                           SIMPLE_FRAG_FILE);
        }
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase,
            Config const &config)
    {
        auto parameters =
                dynamic_cast<renderDetails::ParametersNormalMap*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        auto rd = std::make_shared<RenderDetailsGL>(gameRequester, surfaceDetails->surfaceWidth,
                                                    surfaceDetails->surfaceHeight, surfaceDetails->useIntTexture);

        auto cod = std::make_shared<CommonObjectDataGL>(
                config,
                parameters->widthAtDepth,
                parameters->heightAtDepth);

        return createReference(rd, cod);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &inGameRequester,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase,
            Config const &config)
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

        if (rd->m_usesIntSurface != surfaceDetails->useIntTexture) {
            rd->m_usesIntSurface = surfaceDetails->useIntTexture;
            rd->loadPipeline(inGameRequester);
        }

        auto parameters =
                dynamic_cast<renderDetails::ParametersNormalMap*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        auto cod = std::make_shared<CommonObjectDataGL>(
                config,
                parameters->widthAtDepth,
                parameters->heightAtDepth);

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
                        glm::mat4 const & modelMatrix) -> std::shared_ptr<renderDetails::DrawObjectDataGL>
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
        // set the shader to use
        glUseProgram(m_programID);
        checkGraphicsError();
        glCullFace(GL_BACK);
        checkGraphicsError();

        auto projView = commonObjectData->getProjViewForLevel();

        auto cod = dynamic_cast<CommonObjectDataGL*>(commonObjectData.get());

        GLint MatrixID;

        // the projection matrix
        MatrixID = glGetUniformLocation(m_programID, "proj");
        checkGraphicsError();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(projView.first)[0][0]);
        checkGraphicsError();

        // the view matrix
        MatrixID = glGetUniformLocation(m_programID, "view");
        checkGraphicsError();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(projView.second)[0][0]);
        checkGraphicsError();

        MatrixID = glGetUniformLocation(m_programID, "model");
        checkGraphicsError();

        GLint normalMatrixID = glGetUniformLocation(m_programID, "normalMatrix");
        checkGraphicsError();

        for (auto it = beginZValRefs; it != endZValRefs; it++) {
            auto drawObj = drawObjTable->drawObject(it->drawObjectReference);
            auto modelData = drawObj->modelData();

            auto objData = drawObj->objData(it->drawObjectDataReference.get());
            auto modelMatrix = objData->modelMatrix(modelMatrixID);

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
            checkGraphicsError();

            glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelMatrix));
            glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, &normalMatrix[0][0]);
            checkGraphicsError();

            drawVertices(m_programID, modelData, true);
        }
    }

    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL, Config> registerGL;

} // namespace normalMap