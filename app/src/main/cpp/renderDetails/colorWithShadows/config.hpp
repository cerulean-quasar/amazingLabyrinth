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
#ifndef AMAZING_LABYRINTH_COLORWITHSHADOWS_CONFIG_HPP
#define AMAZING_LABYRINTH_COLORWITHSHADOWS_CONFIG_HPP

#include <glm/glm.hpp>

#include "../../levelTracker/levelTracker.hpp"

namespace textureWithShadows {
    // todo: get these constants from a config file.
    struct Config {
        static float constexpr const viewAngle = 3.1415926f/4.0f;
        static float constexpr const nearPlane = 0.5f;
        static float constexpr const farPlane = 5.0f;
        static glm::vec3 constexpr const viewPoint{0.0f, 0.0f, 1.0f};
        static glm::vec3 constexpr const lightingSource{1.0f, 1.0f, 1.5f};
        static glm::vec3 constexpr const lookAt{0.0f, 0.0f, levelTracker::Loader::m_maxZLevel};
        static glm::vec3 constexpr const up{0.0f, 1.0f, 0.0f};
    };

}

#endif // AMAZING_LABYRINTH_COLORWITHSHADOWS_CONFIG_HPP
