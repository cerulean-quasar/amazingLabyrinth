#version 100
precision mediump float;

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

varying vec3 fragColor;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec3 fragPosition;

uniform sampler2D texSampler;
uniform vec3 lightPos;

void main() {
    // diffuse
    vec3 lightVector = lightPos - fragPosition;
    vec3 lightDirection = normalize(lightVector);
    float diff = max(dot(fragNormal, lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    float falloff = 100.0*dot(lightVector, lightVector);
    gl_FragColor = vec4((fragColor + diffuse)/falloff, 1.0) * texture2D(texSampler, fragTexCoord);
}
