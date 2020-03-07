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

/*
 * get the projection matrix and make all differences between Vulkan and OpenGL into parameters
 * instead of using #defines.  The relevant differences are:
 *
 * (1) the y axis is inverted in Vulkan compared to OpenGL.  I fix this by changing the projection
 * matrix to invert the y coordinate when running Vulkan.
 *
 * (2) the depth is 0.0 to 1.0 in Vulkan and -1.0 to 1.0 in OpenGL.  This is handled by a #define
 * before including glm headers.  If GLM_FORCE_DEPTH_ZERO_TO_ONE is defined before including the
 * headers then the depth is 0.0 to 1.0 in the projection matrices.
 *
 * Notes: all angles are in radians
 */

#include <glm/glm.hpp>

glm::mat4 getPerspectiveMatrix(
        float viewAngle, /* radians */
        float aspectRatio,
        float nearPlane,
        float farPlane,
        bool invertY,
        bool depth0to1);

glm::mat4 getOrthoMatrix(
        float minusX,
        float plusX,
        float minusY,
        float plusY,
        float nearPlane,
        float farPlane,
        bool invertY,
        bool depth0to1);
