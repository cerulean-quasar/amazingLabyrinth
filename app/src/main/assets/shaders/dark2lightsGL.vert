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

uniform mat4 model;
uniform mat4 projView;
uniform mat4 normalMatrix;
uniform mat4 projViewLightBallUp;
uniform mat4 projViewLightBallRight;
uniform mat4 projViewLightBallDown;
uniform mat4 projViewLightBallLeft;
uniform mat4 projViewLightHoleUp;
uniform mat4 projViewLightHoleRight;
uniform mat4 projViewLightHoleDown;
uniform mat4 projViewLightHoleLeft;

attribute vec3 inPosition;
attribute vec3 inColor;
attribute vec2 inTexCoord;
attribute vec3 inNormal;

varying vec3 fragColor;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec3 fragPosition;
varying vec4 fragPosBallLightSpaceUp;
varying vec4 fragPosBallLightSpaceRight;
varying vec4 fragPosBallLightSpaceDown;
varying vec4 fragPosBallLightSpaceLeft;
varying vec4 fragPosHoleLightSpaceUp;
varying vec4 fragPosHoleLightSpaceRight;
varying vec4 fragPosHoleLightSpaceDown;
varying vec4 fragPosHoleLightSpaceLeft;

void main() {
    fragColor = inColor;
    fragTexCoord = inTexCoord;

    /* The transpose and inverse functions are not available
       in GLSL 100, so we passed in the normal matrix. */
    fragNormal = normalize(mat3(normalMatrix) * inNormal);

    vec4 fragPos = model * vec4(inPosition, 1.0);
    fragPosition = fragPos.xyz / fragPos.w;
    fragPosBallLightSpaceUp = projViewLightBallUp * fragPos;
    fragPosBallLightSpaceRight = projViewLightBallRight * fragPos;
    fragPosBallLightSpaceDown = projViewLightBallDown * fragPos;
    fragPosBallLightSpaceLeft = projViewLightBallLeft * fragPos;
    fragPosHoleLightSpaceUp = projViewLightHoleUp * fragPos;
    fragPosHoleLightSpaceRight = projViewLightHoleRight * fragPos;
    fragPosHoleLightSpaceDown = projViewLightHoleDown * fragPos;
    fragPosHoleLightSpaceLeft = projViewLightHoleLeft * fragPos;
    gl_Position = projView * fragPos;
}
