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

#include "renderDetailsData.hpp"

class RenderDetailsDataGL : public RenderDetailsData {
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
            std::shared_ptr<TextureData> &textureData) override;
};

#endif // AMAZING_LABYRINTH_RENDER_DETAILS_DATA_GL_HPP
