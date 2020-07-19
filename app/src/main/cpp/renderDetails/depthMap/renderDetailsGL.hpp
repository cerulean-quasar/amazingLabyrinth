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
#ifndef AMAZING_LABYRINTH_SHADOWS_RENDER_DETAILS_DATA_GL_HPP
#define AMAZING_LABYRINTH_SHADOWS_RENDER_DETAILS_DATA_GL_HPP

#include <memory>

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../renderDetailsGL.hpp"
#include "../renderDetails.hpp"
#include "config.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"
#include "../../renderLoader/renderLoaderGL.hpp"

namespace depthMap {
    class RenderDetailsGL;
    class CommonObjectDataGL : public renderDetails::CommonObjectDataOrtho {
        friend RenderDetailsGL;
    public:
        std::pair<glm::mat4, glm::mat4> getProjViewForLevel() override {
            return std::make_pair<glm::mat4, glm::mat4>(
                    getOrthoMatrix(m_minusX, m_plusX, m_minusY, m_plusY,
                                   m_nearPlane, m_farPlane, false, false),
                    view());
        }

        glm::vec3 getLightSource() override {
            return viewPoint();
        }

        glm::mat4 getViewLightSource() override {
            return view();
        }

        float nearestDepth() { return m_nearestDepth; }
        float farthestDepth() { return m_farthestDepth; }

        ~CommonObjectDataGL() override = default;
    private:
        float m_nearestDepth;
        float m_farthestDepth;

        CommonObjectDataGL(
                float inNearestDepth,
                float inFarthestDepth,
                Config config,
                float width,
                float height)
                : CommonObjectDataOrtho(
                    -width/2, width/2, -height/2, height/2,
                    config.nearPlane, config.farPlane, config.viewPoint, config.lookAt, config.up)
        {}
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectDataGL {
    public:
        glm::mat4 modelMatrix(uint32_t) override { return m_modelMatrix; }

        void update(glm::mat4 const &modelMatrix) override {
            m_modelMatrix = modelMatrix;
        }

        DrawObjectDataGL(glm::mat4 const &inModelMatrix)
                : m_modelMatrix{inModelMatrix}
        {}

        ~DrawObjectDataGL() override = default;
    private:
        glm::mat4 m_modelMatrix;
    };

    class RenderDetailsGL : public renderDetails::RenderDetailsGL {
    public:
        static char const *name() { return shadowsRenderDetailsName; }

        static renderDetails::ReferenceGL loadNew(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                renderDetails::ParametersGL const &parameters,
                Config const &config);

        static renderDetails::ReferenceGL loadExisting(
                std::shared_ptr<GameRequester> const &gameRequester,
                std::shared_ptr<RenderLoaderGL> const &renderLoader,
                std::shared_ptr<renderDetails::RenderDetailsGL> rdBase,
                renderDetails::ParametersGL const &parameters,
                Config const &config);

        void draw(
                uint32_t modelMatrixID,
                std::shared_ptr<renderDetails::CommonObjectData> const &commonObjectData,
                std::shared_ptr<levelDrawer::DrawObjectTableGL> const &drawObjTable,
                std::vector<size_t> const &drawObjectsIndices) override;

        bool overrideClearColor(glm::vec4 &clearColor) override {
            clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            return true;
        }

        ~RenderDetailsGL() override = default;

    private:
        static char constexpr const *LINEAR_DEPTH_VERT_FILE ="shaders/linearDepthGL.vert";
        static char constexpr const *SIMPLE_FRAG_FILE = "shaders/simpleGL.frag";

        GLuint m_depthProgramID;

        static renderDetails::ReferenceGL createReference(
                std::shared_ptr<renderDetails::RenderDetailsGL> rd,
                std::shared_ptr<CommonObjectDataGL> cod);

        RenderDetailsGL(std::shared_ptr<GameRequester> const &inGameRequester,
                        uint32_t inWidth, uint32_t inHeight);
    };
}
#endif // AMAZING_LABYRINTH_SHADOWS_RENDER_DETAILS_DATA_GL_HPP
