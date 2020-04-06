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

#include <glm/gtc/matrix_transform.hpp>

#include "mathGraphics.hpp"

glm::mat4 getDepthMinus1to1Matrix() {
    glm::mat4 matrix(1.0f);
    matrix[2][2] = 2.0f;
    matrix[3][2] = -1.0f;

    return matrix;
}

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
    glm::mat4 proj = glm::perspective(viewAngle, aspectRatio, nearPlane, farPlane);
    if (!depth0to1) {
        proj = getDepthMinus1to1Matrix() * proj;
    }

    if (invertY) {
        proj[1][0] *= -1.0f;
        proj[1][1] *= -1.0f;
        proj[1][2] *= -1.0f;
        proj[1][3] *= -1.0f;
    }

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
    glm::mat4 proj = glm::ortho(minusX, plusX, minusY, plusY, nearPlane, farPlane);
    if (!depth0to1) {
        proj = getDepthMinus1to1Matrix() * proj;
    }

    if (invertY) {
        proj[1][0] *= -1.0f;
        proj[1][1] *= -1.0f;
        proj[1][2] *= -1.0f;
        proj[1][3] *= -1.0f;
    }

    return proj;
}

float transformRange(
        float colorValue,
        float fromLowest,
        float fromHighest,
        float toLowest,
        float toHighest)
{
    float z = (colorValue - fromLowest) *(toHighest - toLowest)/(fromHighest - fromLowest) + toLowest;
    return z;
}

template <>
float convertColor<unsigned char>(unsigned char color, bool transform, float fromLowest, float fromHighest, float toLowest, float toHighest) {
    float ret = static_cast<float>(color) / 255.0f;
    if (transform) {
        ret = transformRange(ret, fromLowest, fromHighest, toLowest, toHighest);
    }
    return ret;
}