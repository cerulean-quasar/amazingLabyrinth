#version 100
precision mediump float;

/**
 * Copyright 2023 Cerulean Quasar. All Rights Reserved.
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

varying vec3 fragColor;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec3 fragPosition;
varying vec4 fragPosLightSpace;

uniform sampler2D texSampler;
uniform sampler2D texShadowMap;
uniform vec3 lightPos;

float ShadowCalculation(vec4 pos) {
    /* perspective divide: transform clip space coordinates from range: [-w, w] to [-1, 1]. */
    vec3 projCoords = pos.xyz/pos.w;

    /* the depth buffer is using coordinates in the range: [-1, 1] switch to coordinates between 0 and 1 */
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }
    float closestDepth = texture2D(texShadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;
    float bias = 0.001;
    float shadow = currentDepth - bias > closestDepth ? 0.6 : 0.0;

    return shadow;
}

void main() {
    // diffuse
    vec3 lightDirection = normalize(lightPos - fragPosition);
    float diff = max(dot(fragNormal, lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // calculate shadows
    float shadow = ShadowCalculation(fragPosLightSpace);

    gl_FragColor = vec4(diffuse*(1.0 - shadow), 1.0) * texture2D(texSampler, fragTexCoord);
}
