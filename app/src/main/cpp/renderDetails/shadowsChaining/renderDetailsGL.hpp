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
#ifndef AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_GL_HPP
#define AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_GL_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "config.hpp"
#include "../shadows/renderDetailsGL.hpp"
#include "../objectWithShadows/renderDetailsGL.hpp"
#include "../../graphicsGL.hpp"
#include "../../renderLoader/renderLoaderGL.hpp"

namespace shadowsChaining {
    enum {
        MODEL_MATRIX_ID_MAIN,
        MODEL_MATRIX_ID_SHADOWS
    };

    class CommonObjectDataGL : public renderDetails::CommonObjectData {
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            return m_objectWithShadowsCOD->getProjViewForLevel();
        }

        glm::vec3 getLightSource() override {
            return m_shadowsCOD->viewPoint();
        }

        glm::mat4 getViewLightSource() override {
            return m_shadowsCOD->view();
        }

        ~CommonObjectDataGL() override = default;
    private:
        std::shared_ptr<objectWithShadows::CommonObjectDataGL> m_objectWithShadowsCOD;
        std::shared_ptr<shadows::CommonObjectDataGL> m_shadowsCOD;

        CommonObjectDataGL(
                std::shared_ptr<objectWithShadows::CommonObjectDataGL> inObjectWithShadowsCOD,
                std::shared_ptr<shadows::CommonObjectDataGL> inShadowsCOD)
                : CommonObjectData(),
                  m_objectWithShadowsCOD(std::move(inObjectWithShadowsCOD)),
                  m_shadowsCOD(std::move(inShadowsCOD))
        {}
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectDataGL {
    public:
        glm::mat4 modelMatrix(uint32_t id) override {
            if (id == MODEL_MATRIX_ID_MAIN) {
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
                std::shared_ptr<objectWithShadows::DrawObjectDataGL> mainDOD,
                std::shared_ptr<shadows::DrawObjectDataGL> shadowsDOD)
                : renderDetails::DrawObjectDataGL{},
                  m_mainDOD{std::move(mainDOD)},
                  m_shadowsDOD{std::move(shadowsDOD)}
        {}

        ~DrawObjectDataGL() override = default;
    private:
        std::shared_ptr<objectWithShadows::DrawObjectDataGL> m_mainDOD;
        std::shared_ptr<shadows::DrawObjectDataGL> m_shadowsDOD;
    };

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        std::string nameString() override { return name(); }
        static char const *name() { return shadowsChainingRenderDetailsName; }
        static renderDetails::ReferenceGL loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                std::shared_ptr<renderDetails::Parameters> const &parameters,
                Config const &config);

        static renderDetails::ReferenceGL loadExisting(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
                std::shared_ptr<renderDetails::Parameters> const &parameters,
                Config const &config);

        void preMainDraw(
                uint32_t modelMatrixID,
                renderDetails::CommonObjectDataList const &commonObjectDataList,
                renderDetails::DrawObjectTableList const &drawObjTableList,
                renderDetails::IndicesForDrawList const &drawObjectsIndicesList) override;

        void draw(
                uint32_t modelMatrixID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<renderDetails::DrawObjectTableGL> const &drawObjTable,
                std::vector<size_t> const &drawObjectsIndices) override;

        RenderDetailsGL(bool useIntTexture, uint32_t inWidth, uint32_t inHeight)
                : renderDetails::RenderDetailsGL{inWidth, inHeight},
                  m_useIntTexture(useIntTexture)
        {}

        ~RenderDetailsGL() override = default;

    private:
        bool m_useIntTexture;
        std::shared_ptr<graphicsGL::Framebuffer> m_framebufferShadows;
        std::shared_ptr<renderDetails::RenderDetailsGL> m_shadowsRenderDetails;
        std::shared_ptr<renderDetails::RenderDetailsGL> m_objectWithShadowsRenderDetails;

        static renderDetails::ReferenceGL createReference(
                std::shared_ptr<renderDetails::RenderDetailsGL> rd,
                renderDetails::ReferenceGL const &refObjectWithShadows,
                renderDetails::ReferenceGL const &refShadows);

        // Initialize framebuffer for shadow mapping.
        static void createFramebuffer(RenderDetailsGL *rd);
    };
}
#endif // AMAZING_LABYRINTH_OBJECTWITHSHADOWS_RENDER_DETAILS_GL_HPP
