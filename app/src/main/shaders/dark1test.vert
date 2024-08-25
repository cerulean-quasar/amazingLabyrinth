/**
 * Copyright 2024 Cerulean Quasar. All Rights Reserved.
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
    mat4 model;
} ubo;

layout(set = 0, binding = 1) uniform CommonUBO {
    mat4 projView;
    mat4 projViewLight1Up;
    mat4 projViewLight1Left;
    mat4 projViewLight1Down;
    mat4 projViewLight1Right;
} cubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPosition;
layout(location = 4) out vec4 fragPosLightSpace1Up;
layout(location = 5) out vec4 fragPosLightSpace1Left;
layout(location = 6) out vec4 fragPosLightSpace1Down;
layout(location = 7) out vec4 fragPosLightSpace1Right;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = normalize(mat3(transpose(inverse(ubo.model))) * inNormal);
    vec4 fragPos = ubo.model * vec4(inPosition, 1.0);
    fragPosition = fragPos.xyz/ fragPos.w;
    fragPosLightSpace1Up = cubo.projViewLight1Up * fragPos;
    fragPosLightSpace1Left = cubo.projViewLight1Left * fragPos;
    fragPosLightSpace1Down = cubo.projViewLight1Down * fragPos;
    fragPosLightSpace1Right = cubo.projViewLight1Right * fragPos;

    gl_Position = cubo.projView * fragPos;
}
