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
#ifndef AMAZING_LABYRINTH_SHADOWSCHAINING_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_SHADOWSCHAINING_RENDER_DETAILS_GL_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "../shadows/renderDetailsGL.hpp"
#include "../objectWithShadows/renderDetailsGL.hpp"
#include "../../graphicsGL.hpp"
#include "../../renderLoader/renderLoaderGL.hpp"

namespace shadowsChaining {
    class CommonObjectDataGL : public renderDetails::CommonObjectData {
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() {
            return m_objectWithShadowsCOD->getProjViewForLevel();
        }

        std::shared_ptr<objectWithShadows::CommonObjectDataGL> const &objectWithShadowsCOD()
        {
            return m_objectWithShadowsCOD;
        }

        std::shared_ptr<shadows::CommonObjectDataGL> const &shadowsCOD() { return m_shadowsCOD; }

        CommonObjectDataGL(
                std::shared_ptr<renderDetails::CommonObjectDataBase> inObjectWithShadowsCOD,
                std::shared_ptr<renderDetails::CommonObjectDataBase> inShadowsCOD)
                // nearPlane and farPlane are unused in shadowsChaining COD
                : CommonObjectData(renderDetails::Parameters{}),
                  m_objectWithShadowsCOD(std::dynamic_pointer_cast<objectWithShadows::CommonObjectDataGL>(inObjectWithShadowsCOD)),
                  m_shadowsCOD(std::dynamic_pointer_cast<shadows::CommonObjectDataGL>(inShadowsCOD))
        {}

        ~CommonObjectDataGL() override = default;
    private:
        std::shared_ptr<objectWithShadows::CommonObjectDataGL> m_objectWithShadowsCOD;
        std::shared_ptr<shadows::CommonObjectDataGL> m_shadowsCOD;
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

        RenderDetailsGL(renderDetails::Description description, bool useIntSurface, uint32_t inWidth, uint32_t inHeight)
                : renderDetails::RenderDetailsGL{std::move(description), inWidth, inHeight, useIntSurface}
        {
            createFramebuffer();
        }

        ~RenderDetailsGL() override = default;

    private:
        std::shared_ptr<graphicsGL::Framebuffer> m_framebufferShadows;
        std::shared_ptr<renderDetails::RenderDetailsGL> m_shadowsRenderDetails;
        std::shared_ptr<renderDetails::RenderDetailsGL> m_objectWithShadowsRenderDetails;

        static renderDetails::ReferenceGL createReference(
                std::shared_ptr<renderDetails::RenderDetailsGL> rd,
                renderDetails::ReferenceGL const &refObjectWithShadows,
                renderDetails::ReferenceGL const &refShadows);

        // Initialize framebuffer for shadow mapping.
        void createFramebuffer();
    };
}
#endif // AMAZING_LABYRINTH_SHADOWSCHAINING_RENDER_DETAILS_GL_HPP
