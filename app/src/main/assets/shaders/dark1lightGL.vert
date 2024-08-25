#version 100
precision mediump float;

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

uniform mat4 model;
uniform mat4 projView;
uniform mat4 normalMatrix;
uniform mat4 projViewLight1Up;
uniform mat4 projViewLight1Left;
uniform mat4 projViewLight1Down;
uniform mat4 projViewLight1Right;

attribute vec3 inPosition;
attribute vec3 inColor;
attribute vec2 inTexCoord;
attribute vec3 inNormal;

varying vec3 fragColor;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec3 fragPosition;
varying vec4 fragPosLightSpace1Up;
varying vec4 fragPosLightSpace1Left;
varying vec4 fragPosLightSpace1Down;
varying vec4 fragPosLightSpace1Right;

void main() {
    fragColor = inColor;
    fragTexCoord = inTexCoord;

    /* The transpose and inverse functions are not available
       in GLSL 100, so we passed in the normal matrix. */
    fragNormal = normalize(mat3(normalMatrix) * inNormal);

    vec4 fragPos = model * vec4(inPosition, 1.0);
    fragPosition = fragPos.xyz / fragPos.w;
    fragPosLightSpace1Up = projViewLight1Up * fragPos;
    fragPosLightSpace1Left = projViewLight1Left * fragPos;
    fragPosLightSpace1Down = projViewLight1Down * fragPos;
    fragPosLightSpace1Right = projViewLight1Right * fragPos;
    gl_Position = projView * fragPos;
}
