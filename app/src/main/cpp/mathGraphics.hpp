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
#ifndef AMAZING_LABYRINTH_MATH_GRAPHICS
#define AMAZING_LABYRINTH_MATH_GRAPHICS
#include <vector>
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

std::pair<float, float> getWidthHeight(
        float depth,
        glm::mat4 const &proj,
        glm::mat4 const &view);

float transformRange(
        float colorValue,
        float fromLowest,
        float fromHighest,
        float toLowest,
        float toHighest);

template <typename inputDataType>
float convertColor(
        inputDataType color,
        bool transform,
        float toLowest,
        float toHighest)
{
    float ret = static_cast<float>(color);
    if (transform) {
        ret = transformRange(ret, 0.0f, static_cast<float>(std::numeric_limits<inputDataType>::max()), toLowest, toHighest);
    }
    return ret;
}

template <>
float convertColor<float>(
        float color,
        bool transform,
        float toLowest,
        float toHighest);

template <typename inputDataType>
void bitmapToDepthMap(
        std::vector<inputDataType> const &texture,
        float farthestDepth,
        float nearestDepth,
        uint32_t surfaceWidth,
        uint32_t surfaceHeight,
        uint32_t step,  /* how many colors per data point */
        bool invertY,
        std::vector<float> &depthMap)
{
    depthMap.resize(surfaceWidth * surfaceHeight);
    if (invertY) {
        for (size_t i = 0; i < surfaceWidth; i++) {
            for (size_t j = 0; j < surfaceHeight; j++) {
                float red = convertColor<inputDataType>(
                        texture[((surfaceHeight - 1 - j) * surfaceWidth + i) * step], true, farthestDepth, nearestDepth);
                depthMap[j * surfaceWidth + i] = red;
            }
        }
    } else {
        for (size_t i = 0; i < depthMap.size(); i++) {
            float red = convertColor<inputDataType>(texture[i * step], true, farthestDepth, nearestDepth);
            depthMap[i] = red;
        }
    }
}

template <typename inputDataType>
void bitmapToNormals(
        std::vector<inputDataType> const &texture,
        uint32_t surfaceWidth,
        uint32_t surfaceHeight,
        uint32_t step,  /* how many colors per data point */
        bool invertY,
        std::vector<glm::vec3> &normalMap)
{
    if (step < 3) {
        throw std::runtime_error("there must be at least three components in the normal map");
    }
    normalMap.resize(surfaceWidth * surfaceHeight);
    if (invertY) {
        for (size_t i = 0; i < surfaceWidth; i++) {
            for (size_t j = 0; j < surfaceHeight; j++) {
                float x = convertColor<inputDataType>(
                        texture[((surfaceHeight - 1 - j) * surfaceWidth + i) * step], true, -1.0f, 1.0f);
                float y = convertColor<inputDataType>(
                        texture[((surfaceHeight - 1 - j) * surfaceWidth + i) * step + 1], true, -1.0f, 1.0f);
                float z = convertColor<inputDataType>(
                        texture[((surfaceHeight - 1 - j) * surfaceWidth + i) * step + 2], true, -1.0f, 1.0f);
                normalMap[j * surfaceWidth + i] = glm::vec3{x, y, z};
            }
        }
    } else {
        for (size_t i = 0; i < normalMap.size(); i++) {
            float x = convertColor<inputDataType>(texture[i * step], true, -1.0f, 1.0f);
            float y = convertColor<inputDataType>(texture[i * step + 1], true, -1.0f, 1.0f);
            float z = convertColor<inputDataType>(texture[i * step + 2], true, -1.0f, 1.0f);
            normalMap[i] = glm::vec3{x, y, z};
        }
    }
}

#endif