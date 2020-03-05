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

#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>

#include "mathGraphics.hpp"


/*
 * get a perspective matrix.  viewAngle is in radians.
 */
glm::mat4 getPerspectiveMatrix(
        float viewAngle,
        float aspectRatio,
        float nearPlane,
        float farPlane,
        bool invertY,
        bool depth0to1)
{
    glm::mat4 proj;
    if (depth0to1) {
        proj = getPerspectiveMatrixDepth0to1(viewAngle, aspectRatio, nearPlane, farPlane);
    } else {
        proj = getPerspectiveMatrixDepthMinus1to1(viewAngle, aspectRatio, nearPlane, farPlane);
    }

    if (invertY) {
        proj[1][1] *= -1;
    }

    return proj;
}

glm::mat4 getPerspectiveMatrixDepthMinus1to1(
        float viewAngle,
        float aspectRatio,
        float nearPlane,
        float farPlane)
{
    /* perspective matrix: takes the perspective projection, the aspect ratio, near and far
     * view planes.
     */
    glm::mat4 proj = glm::perspective(viewAngle, aspectRatio, nearPlane, farPlane);
    return proj;
}

glm::mat4 getOrthoMatrix(
        float minusX,
        float plusX,
        float minusY,
        float plusY,
        float nearPlane,
        float farPlane,
        bool invertY,
        bool depth0to1)
{
    glm::mat4 proj;
    if (depth0to1) {
        proj = getOrthoMatrixDepth0to1(minusX, plusX, minusY, plusY, nearPlane, farPlane);
    } else {
        proj = getOrthoMatrixDepthMinus1to1(minusX, plusX, minusY, plusY, nearPlane, farPlane);
    }

    if (invertY) {
        proj[1][1] *= -1;
    }

    return proj;
}

glm::mat4 getOrthoMatrixDepthMinus1to1(
        float minusX,
        float plusX,
        float minusY,
        float plusY,
        float nearPlane,
        float farPlane)
{
    return glm::ortho(minusX, plusX, minusY, plusY, nearPlane, farPlane);
}