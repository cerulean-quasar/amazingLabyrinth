#version 100
precision highp float;

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

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 normalMatrix;

attribute vec3 inPosition;
attribute vec3 inColor;
attribute vec2 inTexCoord;
attribute vec3 inNormal;

varying vec3 fragColor;
varying vec3 fragNormal;

void main() {
    vec4 pos = proj * view * model * vec4(inPosition, 1.0);
    vec4 normalVec = normalMatrix * vec4(inNormal, 1.0);
    if (normalVec.z/normalVec.w < 0.0) {
        gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
    } else {
        gl_Position = vec4(pos.x * normalVec.w, pos.y * normalVec.w, normalVec.z * pos.w, normalVec.w * pos.w);
    }
    fragColor = normalize(normalVec.xyz/normalVec.w) * 0.5 + vec3(0.5, 0.5, 0.5);
}
