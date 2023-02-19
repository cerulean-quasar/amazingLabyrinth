/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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

    RenderDetailsGL::RenderDetailsGL(
            char const *name,
            char const *vertexShader,
            char const *fragShader,
            std::shared_ptr<GameRequester> const &inGameRequester,
            uint32_t inWidth,
            uint32_t inHeight,
            bool usesIntSurface)
            : renderDetails::RenderDetailsGL(inWidth, inHeight, usesIntSurface),
              m_renderDetailsName{name},
              m_depthProgramID{loadShaders(inGameRequester, vertexShader, fragShader)}
    {}

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

        if (shaders.size() != 2) {
            throw std::runtime_error("Invalid number of shaders passed into Render Details.");
        }

        auto rd = std::make_shared<RenderDetailsGL>(
                name, shaders[0], shaders[1],
                gameRequester, surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight,
                surfaceDetails->useIntTexture);

        auto cod = std::make_shared<CommonObjectDataGL>(*parameters,
                surfaceDetails->surfaceWidth / static_cast<float>(surfaceDetails->surfaceHeight));

        return createReference(rd, cod);
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

        auto parameters =
                dynamic_cast<renderDetails::ParametersPerspective*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        if (rd->m_surfaceWidth != surfaceDetails->surfaceWidth ||
            rd->m_surfaceHeight != surfaceDetails->surfaceHeight)
        {
            rd->m_surfaceWidth = surfaceDetails->surfaceWidth;
            rd->m_surfaceHeight = surfaceDetails->surfaceHeight;
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
        auto cod = dynamic_cast<CommonObjectDataGL *>(commonObjectData.get());
        if (!cod) {
            throw std::runtime_error("Invalid common object data type");
        }

        // set the shader to use
        glUseProgram(m_depthProgramID);
        checkGraphicsError();
        glCullFace(GL_FRONT);
        checkGraphicsError();

        auto projView = cod->getProjViewForLevel();

        GLint MatrixID;

        // the projection matrix * the view matrix
        glm::mat4 projTimesView = projView.first * projView.second;
        MatrixID = glGetUniformLocation(m_depthProgramID, "projView");
        checkGraphicsError();
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projTimesView[0][0]);
        checkGraphicsError();

        MatrixID = glGetUniformLocation(m_depthProgramID, "model");
        checkGraphicsError();

        for (auto it = beginZValRefs; it != endZValRefs; it++) {
            auto drawObj = drawObjTable->drawObject(it->drawObjectReference);
            auto modelData = drawObj->modelData();

            auto objData = drawObj->objData(it->drawObjectDataReference.get());
            auto modelMatrix = objData->modelMatrix(modelMatrixID);

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
            checkGraphicsError();

            drawVertices(m_depthProgramID, modelData);
        }
    }

    char constexpr const *DEPTH_VERT_FILE = "shaders/depthShaderGL.vert";
    char constexpr const *SIMPLE_FRAG_FILE = "shaders/simpleGL.frag";
    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL> registerGL(
        shadowsRenderDetailsName,
        std::vector<char const *>{DEPTH_VERT_FILE, SIMPLE_FRAG_FILE});

} // namespace shadows
