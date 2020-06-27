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
#ifndef AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_VULKAN_HPP
#define AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_VULKAN_HPP

#include <glm/glm.hpp>

struct CommonUBO {
    glm::mat4 viewLightMatrix;
    glm::mat4 view;
    glm::mat4 proj;
};

#endif // AMAZING_LABYRINTH_LEVEL_DRAWER_COMMON_VULKAN_HPP