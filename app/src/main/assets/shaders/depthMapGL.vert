#version 100
precision highp float;

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

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float nearestDepth;
uniform float farthestDepth;

attribute vec3 inPosition;
attribute vec3 inColor;
attribute vec2 inTexCoord;
attribute vec3 inNormal;

varying vec3 fragColor;

void main() {
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    gl_Position.z = -gl_Position.z;
    vec4 pos = model * vec4(inPosition, 1.0);
    float z = (pos.z/pos.w - farthestDepth)/(nearestDepth - farthestDepth);
    if (z > 1.0) {
        z = 1.0;
    } else if (z < 0.0) {
        z = 0.0;
    }
    fragColor = vec3(z, z, z);
}
