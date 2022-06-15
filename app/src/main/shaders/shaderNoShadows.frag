/**
 * Copyright 2022 Cerulean Quasar. All Rights Reserved.
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

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosition;

layout(binding = 2) uniform sampler2D texSampler;
layout(set = 0, binding = 3) uniform UniformBufferObject {
    vec3 pos;
} light;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDirection = normalize(light.pos - fragPosition);
    float diff = max(dot(fragNormal, lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    outColor = vec4(fragColor + diffuse, 1.0) * texture(texSampler, fragTexCoord);
}
