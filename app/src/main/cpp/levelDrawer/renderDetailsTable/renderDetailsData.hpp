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
#ifndef AMAZING_LABYRINTH_RENDER_DETAILS_DATA_HPP
#define AMAZING_LABYRINTH_RENDER_DETAILS_DATA_HPP

#include <memory>

#include "../drawObjectTable/drawObject.hpp"
#include "../../levelTracker/levelTracker.hpp"

class RenderDetailsData {
public:
    glm::mat4 getViewMatrix() {
        return glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
                           glm::vec3(0.0f, 0.0f, levelTracker::Loader::m_maxZLevel), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    virtual glm::mat4 getPerspectiveMatrixForLevel() = 0;
    virtual std::shared_ptr<DrawObjectData> createDrawObjectData(
            std::shared_ptr<TextureData> &textureData) = 0;

    RenderDetailsData(uint32_t inWidth, uint32_t inHeight)
        : m_surfaceWidth{inWidth},
          m_surfaceHeight{inHeight},
          m_lightingSource{1.0f, 1.0f, 1.5f}
    {}

    virtual ~RenderDetailsData() = default;
protected:
    static float constexpr m_perspectiveViewAngle = 3.141593f/4.0f;
    static float constexpr m_perspectiveNearPlane = 0.5f;
    static float constexpr m_perspectiveFarPlane = 5.0f;

    uint32_t m_surfaceWidth;
    uint32_t m_surfaceHeight;
    glm::vec3 m_lightingSource;
};

#endif
