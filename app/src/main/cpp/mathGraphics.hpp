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

float colorValueToDepth(
        float colorValue,
        glm::mat4 const &inverseVP);

template <typename inputDataType>
float convertColor(inputDataType color, bool depth0to1) {
    float ret = static_cast<float>(color);
    if (!depth0to1) {
        ret = ret * 2.0f - 1.0f;
    }
    return ret;
}

template <>
float convertColor<unsigned char>(unsigned char color, bool depth0to1);

template <typename inputDataType>
void bitmapToDepthMap(
        std::vector<inputDataType> const &texture,
        glm::mat4 const &proj,
        glm::mat4 const &view,
        uint32_t surfaceWidth,
        uint32_t surfaceHeight,
        uint32_t step,  /* how many colors per data point */
        bool invertY,
        bool depth0to1,
        std::vector<float> &depthMap)
{
    glm::mat4 invVP = glm::inverse(proj *view);
    depthMap.resize(surfaceWidth * surfaceHeight);
    if (invertY) {
        for (size_t i = 0; i < surfaceWidth; i++) {
            for (size_t j = 0; j < surfaceHeight; j++) {
                float red = convertColor<inputDataType>(
                        texture[((surfaceHeight - 1 - j) * surfaceWidth + i) * step],
                        depth0to1);
                depthMap[j * surfaceWidth + i] = colorValueToDepth(red, invVP);
            }
        }
    } else {
        for (size_t i = 0; i < depthMap.size(); i++) {
            float red = convertColor<inputDataType>(texture[i * step], depth0to1);
            depthMap[i] = colorValueToDepth(red, invVP);
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
                        texture[((surfaceHeight - 1 - j) * surfaceWidth + i) * step], false);
                float y = convertColor<inputDataType>(
                        texture[((surfaceHeight - 1 - j) * surfaceWidth + i) * step + 1], false);
                float z = convertColor<inputDataType>(
                        texture[((surfaceHeight - 1 - j) * surfaceWidth + i) * step + 2], false);
                normalMap[j * surfaceWidth + i] = glm::vec3{x, y, z};
            }
        }
    } else {
        for (size_t i = 0; i < normalMap.size(); i++) {
            float x = convertColor<inputDataType>(texture[i * step], false);
            float y = convertColor<inputDataType>(texture[i * step + 1], false);
            float z = convertColor<inputDataType>(texture[i * step + 2], false);
            normalMap[i] = glm::vec3{x, y, z};
        }
    }
}