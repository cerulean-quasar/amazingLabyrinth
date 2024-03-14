/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
#ifndef AMAZING_LABYRINTH_DARKCHAINING_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_DARKCHAINING_RENDER_DETAILS_GL_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "renderDetails/renderDetails.hpp"
#include "renderDetails/renderDetailsGL.hpp"
#include "renderDetails/renderDetails.hpp"
#include "renderDetails/shadows/renderDetailsGL.hpp"
#include "renderDetails/dark/darkObject/renderDetailsGL.hpp"
#include "graphicsGL.hpp"
#include "renderLoader/renderLoaderGL.hpp"

namespace darkChaining {
    class CommonObjectDataGL : public renderDetails::CommonObjectDataBase {
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() {
            return m_darkObject->getProjViewForLevel();
        }

        std::shared_ptr<darkObject::CommonObjectDataGL> const &darkObject()
        {
            return m_darkObject;
        }

        std::shared_ptr<shadows::CommonObjectDataGL> const &shadowsCOD(size_t i) { return m_shadowsCODs[i]; }

        void update(renderDetails::ParametersBase const &parametersBase) override {
            auto parameters = dynamic_cast<renderDetails::ParametersDark const &>(parametersBase);

            // shadows CODs
            std::vector<glm::mat4> shadowsProjViews;
            for (size_t lightNumber = 0; lightNumber < parameters.numberLightSources(); lightNumber++) {
                for (size_t direction = 0; direction < 4; direction++) {
                    auto parametersShadows = parameters.toShadowsParametersPerspective(
                            lightNumber, direction);

                    // The shadows CODs are stored with the zeroth viewpoint first with the CODs stored
                    // in counterclockwise order starting with the camera pointed in the y direction.
                    // Next the first viewpoint's CODs stored in the same manner.
                    m_shadowsCODs[4 * lightNumber + direction]->update(parametersShadows);

                    glm::mat4 shadowsProjView = parameters.getLightProjView(lightNumber, direction, false, false);
                    shadowsProjViews.push_back(shadowsProjView);
                }
            }

            renderDetails::ParametersDarkObjectGL parametersGame(*parameters.toGamePerspective(), shadowsProjViews);
            m_darkObject->update(parametersGame);
        }

        CommonObjectDataGL(
                std::shared_ptr<renderDetails::CommonObjectDataBase> inDarkObject,
                std::vector<std::shared_ptr<renderDetails::CommonObjectDataBase>> inShadowsCODs)
                : CommonObjectDataBase(), // Base COD
                  m_darkObject(std::dynamic_pointer_cast<darkObject::CommonObjectDataGL>(inDarkObject)),
                  m_shadowsCODs{}
        {
            for (auto const &shadowsCOD : inShadowsCODs) {
                m_shadowsCODs.push_back(std::dynamic_pointer_cast<shadows::CommonObjectDataGL>(shadowsCOD));
            }
        }

        ~CommonObjectDataGL() override = default;
    private:
        std::shared_ptr<darkObject::CommonObjectDataGL> m_darkObject;
        std::vector<std::shared_ptr<shadows::CommonObjectDataGL>> m_shadowsCODs;
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectDataGL {
    public:
        glm::mat4 modelMatrix(uint32_t id) override {
            if (id == renderDetails::MODEL_MATRIX_ID_MAIN) {
                return m_mainDOD->modelMatrix(id);
            } else {
                return m_shadowsDOD->modelMatrix(id);
            }
        }

        void update(glm::mat4 const &modelMatrix) override {
            m_mainDOD->update(modelMatrix);
            m_shadowsDOD->update(modelMatrix);
        }

        DrawObjectDataGL(
                std::shared_ptr<renderDetails::DrawObjectDataGL> mainDOD,
                std::shared_ptr<renderDetails::DrawObjectDataGL> shadowsDOD)
                : renderDetails::DrawObjectDataGL{},
                  m_mainDOD{std::move(mainDOD)},
                  m_shadowsDOD{std::move(shadowsDOD)}
        {}

        ~DrawObjectDataGL() override = default;
    private:
        std::shared_ptr<renderDetails::DrawObjectDataGL> m_mainDOD;
        std::shared_ptr<renderDetails::DrawObjectDataGL> m_shadowsDOD;
    };

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        static renderDetails::ReferenceGL loadNew(
            renderDetails::Description const &description,
            std::vector<char const *> const &shaders,
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::ParametersBase> const &parameters);

        static renderDetails::ReferenceGL loadExisting(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::ParametersBase> const &parameters);

        void preMainDraw(
            uint32_t modelMatrixID,
            levelDrawer::CommonObjectDataList const &commonObjectDataList,
            levelDrawer::DrawObjectTableGList const &drawObjTableList,
            std::set<levelDrawer::ZValueReference> const &starterZValues,
            std::set<levelDrawer::ZValueReference> const &levelZValues,
            std::set<levelDrawer::ZValueReference> const &finisherZValues) override;

        void draw(
            uint32_t modelMatrixID,
            std::shared_ptr<renderDetails::CommonObjectDataBase> const &commonObjectData,
            std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
            std::set<levelDrawer::ZValueReference>::iterator beginZValRefs,
            std::set<levelDrawer::ZValueReference>::iterator endZValRefs) override;

        RenderDetailsGL(renderDetails::Description inDescription, bool useIntSurface, uint32_t inWidth, uint32_t inHeight)
            : renderDetails::RenderDetailsGL{std::move(inDescription), inWidth, inHeight, useIntSurface}
        {
            createFramebuffers();
        }

        ~RenderDetailsGL() override = default;

    private:
        // use less precision for the shadow buffer
        static float constexpr shadowsSizeMultiplier = 0.25f;
        std::vector<std::shared_ptr<graphicsGL::Framebuffer>> m_framebuffersShadows;
        std::shared_ptr<renderDetails::RenderDetailsGL> m_shadowsRenderDetails;
        std::shared_ptr<renderDetails::RenderDetailsGL> m_darkObjectRenderDetails;

        static renderDetails::ReferenceGL createReference(
            std::shared_ptr<renderDetails::RenderDetailsGL> rd,
            renderDetails::ReferenceGL const &refObjectWithShadows,
            renderDetails::ReferenceGL const &refShadows,
            std::vector<std::shared_ptr<renderDetails::CommonObjectDataBase>> shadowsCODs);

        static renderDetails::ReferenceGL loadHelper(
            std::shared_ptr<GameRequester> const &gameRequester,
            std::shared_ptr<RenderLoaderGL> const &renderLoader,
            std::shared_ptr<RenderDetailsGL> rd,
            std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails,
            std::shared_ptr<renderDetails::ParametersBase> const &parametersBase);

        static std::pair<uint32_t, uint32_t> getShadowsFramebufferDimensions(std::pair<uint32_t, uint32_t> const &dimensions) {
            uint32_t width = static_cast<uint32_t>(std::floor(dimensions.first * shadowsSizeMultiplier));

            // use width to make the height of the shadow map image because we do not need something really tall
            // and for most devices the height is much bigger than the width.  Also, multiply by the shadow size multiplier
            // twice so that it is even smaller...
            uint32_t height = static_cast<uint32_t>(std::floor(dimensions.first * shadowsSizeMultiplier * shadowsSizeMultiplier));
            return std::make_pair(width, height);
        }

        static std::shared_ptr<graphicsGL::SurfaceDetails> createShadowSurfaceDetails(
                std::shared_ptr<graphicsGL::SurfaceDetails> const &surfaceDetails)
        {
            graphicsGL::SurfaceDetails shadowSurface{};
            auto wh = getShadowsFramebufferDimensions(std::make_pair(surfaceDetails->surfaceWidth, surfaceDetails->surfaceHeight));
            shadowSurface.surfaceWidth = wh.first;
            shadowSurface.surfaceHeight = wh.second;

            return std::make_shared<graphicsGL::SurfaceDetails>(std::move(shadowSurface));
        }


        // Initialize framebuffer for shadow mapping.
        void createFramebuffers();
    };
}
#endif // AMAZING_LABYRINTH_DARKCHAINING_RENDER_DETAILS_GL_HPP
