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

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 mvp;
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
    float near = 0.1;
    float far = 10.0;
    vec4 pos = ubo.mvp * vec4(inPosition, 1.0);
    vec3 pos3 = pos.xyz / pos.w;
    pos3.z = near * far / (far + pos3.z * (near - far));
    pos3.z = (near - pos3.z)/ (far - near);
    if (pos3.z <= 0.0038) {
        pos3.z = 0.5;
    } else if (pos3.z >= 1.0) {
        pos3.z = 0.5;
    } else {
        pos3.z = 0.0;
    }
    gl_Position = vec4(pos3, 1.0);
//    gl_Position = pos;
//    gl_Position = vec4(pos3.xy, 0.5, 1.0);
    fragColor = gl_Position.zzz;
}
