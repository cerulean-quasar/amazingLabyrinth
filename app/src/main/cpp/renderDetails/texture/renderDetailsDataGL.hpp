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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_DATA_GL_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_DATA_GL_HPP

#include <glm/glm.hpp>

#include <GLES3/gl3.h>

#include "../../levelDrawer/drawObjectTable/drawObjectDataGL.hpp"

#include "../../levelDrawer/renderDetailsTable/renderDetailsCommonGL.hpp"
#include "../../levelDrawer/renderDetailsTable/renderDetailsData.hpp"

class RenderDetailsDataGL : public RenderDetailsData {
public:
    glm::mat4 getPerspectiveMatrixForLevel() override {
        /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
         * view planes.
         */
        return getPerspectiveMatrix(m_perspectiveViewAngle,
                                    m_surfaceWidth / static_cast<float>(m_surfaceHeight),
                                    m_perspectiveNearPlane, m_perspectiveFarPlane,
                                    false, false);
    }

    std::shared_ptr<DrawObjectData> createDrawObjectData(
        std::shared_ptr<TextureData> &textureData,
        glm::mat4 const &modelMatrix) override
    {
        return std::make_shared<DrawObjectDataGL>(modelMatrix);
    }

    RenderDetailsDataGL(std::shared_ptr<GameRequester> const &inGameRequester,
         uint32_t inWidth, uint32_t inHeight);

    ~RenderDetailsDataGL() override = default;
private:
    GLuint m_mainProgramID;
    GLuint m_depthProgramID;
};

#endif // AMAZING_LABYRINTH_RENDER_DETAILS_DATA_GL_HPP
