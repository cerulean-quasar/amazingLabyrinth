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

#include "../basic/renderDetailsCommonGL.hpp"
#include "../basic/renderDetailsData.hpp"
#include "config.hpp"
#include "../../levelDrawer/textureTable/textureLoader.hpp"

namespace shadows {
    class RenderDetailsDataGL;
    class CommonObjectDataGL : public renderDetails::CommonObjectData {
        friend RenderDetailsDataGL;
    public:
        glm::mat4 getPerspectiveMatrixForLevel() override {
            /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
             * view planes.
             */
            return getPerspectiveMatrix(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane,
                                        false, false);
        }
    private:
        CommonObjectDataGL(float aspectRatio, Config const &config)
            : CommonObjectData(config.viewAngle, aspectRatio, config.nearPlane, config.farPlane,
                    config.lightingSource, config.lookAt, config.up)
        {}
    };

    class DrawObjectDataGL : public renderDetails::DrawObjectData {
    public:
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

    class RenderDetailsDataGL : public renderDetails::RenderDetailsData {
    public:
        std::shared_ptr<renderDetails::CommonObjectData> createCommonObjectData(
                Config const &config)
        {
            return std::make_shared<CommonObjectDataGL>(
                    m_surfaceWidth/static_cast<float>(m_surfaceHeight),
                    config);
        }

        std::shared_ptr<renderDetails::DrawObjectData> createDrawObjectData(
                std::shared_ptr<TextureData> const &textureData,
                glm::mat4 const &modelMatrix) override {
            return std::make_shared<DrawObjectDataGL>(modelMatrix);
        }

        RenderDetailsDataGL(std::shared_ptr<GameRequester> const &inGameRequester,
                            uint32_t inWidth, uint32_t inHeight);

        ~RenderDetailsDataGL() override = default;

    private:
        static char constexpr const *DEPTH_VERT_FILE = "shaders/depthShaderGL.vert";
        static char constexpr const *SIMPLE_FRAG_FILE = "shaders/simpleGL.frag";

        GLuint m_depthProgramID;
    };
}
#endif // AMAZING_LABYRINTH_SHADOWS_RENDER_DETAILS_DATA_GL_HPP
