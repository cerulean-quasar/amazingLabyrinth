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
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CommonUniformBufferObject {
    mat4 projView;
    float nearestDepth;
    float farthestDepth;
} cubo;

layout(set = 0, binding = 1) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = cubo.projView * ubo.model * vec4(inPosition, 1.0);
    gl_Position.z = 1.0 - gl_Position.z;
    vec4 pos = ubo.model * vec4(inPosition, 1.0);
    float z = (pos.z/pos.w - cubo.farthestDepth)/(cubo.nearestDepth - cubo.farthestDepth);
    if (z > 1.0) {
        z = 1.0;
    } else if (z < 0.0) {
        z = 0.0;
    }
    fragColor = vec3(z, z, z);
}
