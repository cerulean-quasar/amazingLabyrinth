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

// The following #define makes glm give 0.0f to 1.0f for the depth when computing projection matrices.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
//#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 getPerspectiveMatrixDepth0to1(
        float viewAngle,
        float aspectRatio,
        float nearPlane,
        float farPlane)
{
    /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
     * view planes.
     */
    return glm::perspective(viewAngle, aspectRatio, nearPlane, farPlane);
}

glm::mat4 getOrthoMatrixDepth0to1(
        float minusX,
        float plusX,
        float minusY,
        float plusY,
        float nearPlane,
        float farPlane)
{
    return glm::ortho(minusX, plusX, minusY, plusY, nearPlane, farPlane);
}