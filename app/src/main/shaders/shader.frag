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

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in vec4 fragPosLightSpace;

layout(binding = 2) uniform sampler2D texSampler;
layout(set = 0, binding = 3) uniform UniformBufferObject {
    vec3 pos;
} light;
layout(binding = 4) uniform sampler2D texShadowMap;

layout(location = 0) out vec4 outColor;

float ShadowCalculation(vec4 pos) {
    /* perspective divide: transform clip space coordinates from range: [-w, w] to [-1, 1]. */
    vec3 projCoords = pos.xyz/pos.w;

    /* the depth buffer is using coordinates in the range: [0, 1] */
    projCoords = vec3(projCoords.x * 0.5 + 0.5, 0.5 + projCoords.y * 0.5, projCoords.z);

    if (projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0 || projCoords.z > 1.0) {
        return 1.0;
    }
    float closestDepth = texture(texShadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;
    float bias = 0.001;
    float shadow = currentDepth - bias > closestDepth ? 0.6 : 0.0;
    //float shadow = currentDepth;
    return shadow;
}

void main() {
    vec3 lightDirection = normalize(light.pos - fragPosition);
    float diff = max(dot(fragNormal, lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace);
    //outColor = vec4(vec3(1.0,1.0,1.0)*shadow, 1.0);
    outColor = vec4(fragColor + diffuse*(1.0 - shadow), 1.0) * texture(texSampler, fragTexCoord);
    //outColor = vec4(fragColor + diffuse, 1.0) * texture(texShadowMap, fragTexCoord).r;
}
