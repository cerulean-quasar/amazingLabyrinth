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
varying vec3 fragNormal;
varying vec3 fragPosition;

uniform vec3 lightPos;

void main() {
    // diffuse
    vec3 lightDirection = normalize(lightPos - fragPosition);
    float diff = max(dot(fragNormal, lightDirection), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    gl_FragColor = vec4(vec3(0.2, 0.2, 0.2) + diffuse, 1.0) * vec4(fragColor, 1.0);
}
