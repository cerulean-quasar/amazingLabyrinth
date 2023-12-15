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

#include "../renderDetailsGL.hpp"
#include "renderDetailsGL.hpp"

namespace depthMap {
    RenderDetailsGL::RenderDetailsGL(
            char const *name,
            char const *linearDepthVertShader,
            char const *linearDepthVertShader3,
            char const *simpleFragShader,
            char const *simpleFragShader3,
            std::shared_ptr<GameRequester> const &inGameRequester,
            uint32_t inWidth, uint32_t inHeight, bool useIntSurface)
            : renderDetails::RenderDetailsGL(inWidth, inHeight, useIntSurface),
              m_renderDetailsName{name},
              m_linearDepthVertShader{linearDepthVertShader},
              m_linearDepthVertShader3{linearDepthVertShader3},
              m_simpleFragShader{simpleFragShader},
              m_simpleFragShader3{simpleFragShader3},
              m_depthProgram{}
    {
        loadPipeline(inGameRequester);
    }

    void RenderDetailsGL::loadPipeline(std::shared_ptr<GameRequester> const &inGameRequester) {
        if (m_usesIntSurface) {
            auto vertexShader = cacheShader(inGameRequester, m_linearDepthVertShader3, GL_VERTEX_SHADER);
            auto fragmentShader = cacheShader(inGameRequester, m_simpleFragShader3, GL_FRAGMENT_SHADER);
            m_depthProgram = std::make_shared<renderDetails::GLProgram>(
                    std::vector{std::move(vertexShader), std::move(fragmentShader)});
        } else {
            auto vertexShader = cacheShader(inGameRequester, m_linearDepthVertShader, GL_VERTEX_SHADER);
            auto fragmentShader = cacheShader(inGameRequester, m_simpleFragShader, GL_FRAGMENT_SHADER);
            m_depthProgram = std::make_shared<renderDetails::GLProgram>(
                    std::vector{std::move(vertexShader), std::move(fragmentShader)});
        }
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadNew(
            char const *name,
            std::vector<char const *> const &shaders,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::Parameters> const &parametersBase)
    {
        auto parameters =
            dynamic_cast<renderDetails::ParametersDepthMap*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        if (shaders.size() != 4) {
            throw std::runtime_error("Invalid number of shaders for the Depth Map Render Details.");
        }

        auto rd = std::make_shared<RenderDetailsGL>(
                name, shaders[0], shaders[1], shaders[2], shaders[3],
                gameRequester, surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight,
                surfaceDetails->useIntTexture);

        auto cod = std::make_shared<CommonObjectDataGL>(*parameters);

        return createReference(rd, cod);
    }

    renderDetails::ReferenceGL RenderDetailsGL::loadExisting(
            std::shared_ptr<GameRequester> const &inGameRequester,
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

        if (rd->m_usesIntSurface != surfaceDetails->useIntTexture) {
            rd->m_usesIntSurface = surfaceDetails->useIntTexture;
            rd->loadPipeline(inGameRequester);
        }

        auto parameters =
                dynamic_cast<renderDetails::ParametersDepthMap*>(parametersBase.get());
        if (parameters == nullptr) {
            throw std::runtime_error("Invalid render details parameter type.");
        }

        auto cod = std::make_shared<CommonObjectDataGL>(*parameters);

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
        auto cod = dynamic_cast<CommonObjectDataGL*>(commonObjectData.get());
        if (!cod) {
            throw std::runtime_error("Invalid common object data type");
        }

        // set the shader to use
        GLuint programID = m_depthProgram->programID();
        glUseProgram(programID);
        checkGraphicsError();
        glCullFace(GL_BACK);
        checkGraphicsError();

        auto projView = cod->getProjViewForLevel();

        GLint MatrixID;

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

        GLint loc = glGetUniformLocation(programID, "nearestDepth");
        glUniform1f(loc, cod->nearestDepth());
        checkGraphicsError();

        loc = glGetUniformLocation(programID, "farthestDepth");
        glUniform1f(loc, cod->farthestDepth());
        checkGraphicsError();

        MatrixID = glGetUniformLocation(programID, "model");
        checkGraphicsError();

        for (auto it = beginZValRefs; it != endZValRefs; it++) {
            auto drawObj = drawObjTable->drawObject(it->drawObjectReference);
            auto modelData = drawObj->modelData();

            auto objData = drawObj->objData(it->drawObjectDataReference.get());
            auto modelMatrix = objData->modelMatrix(modelMatrixID);

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
            checkGraphicsError();

            drawVertices(programID, modelData);
        }
    }

    char constexpr const *LINEAR_DEPTH_VERT_GL_FILE = "shaders/linearDepthGL.vert";
    char constexpr const *LINEAR_DEPTH3_VERT_GL_FILE = "shaders/linearDepthGL3.vert";
    char constexpr const *SIMPLE_FRAG_GL_FILE = "shaders/simpleGL.frag";
    char constexpr const *SIMPLE3_FRAG_GL_FILE = "shaders/simpleGL3.frag";
    RegisterGL<renderDetails::RenderDetailsGL, RenderDetailsGL> registerGL(
            depthMapRenderDetailsName,
            std::vector<char const *>{LINEAR_DEPTH_VERT_GL_FILE, LINEAR_DEPTH3_VERT_GL_FILE, SIMPLE_FRAG_GL_FILE, SIMPLE3_FRAG_GL_FILE});

} // namespace depthMap