#version 100
precision mediump float;

/**
 * Copyright 2025 Cerulean Quasar. All Rights Reserved.
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
varying vec4 fragPosLightSpace1Up;
varying vec4 fragPosLightSpace1Left;
varying vec4 fragPosLightSpace1Down;
varying vec4 fragPosLightSpace1Right;

uniform sampler2D texDark1Up;
uniform sampler2D texDark1Left;
uniform sampler2D texDark1Down;
uniform sampler2D texDark1Right;

uniform vec3 lightPos1;

int ShadowCalculation(vec4 pos, sampler2D texSamplerShadows) {
    /* perspective divide: transform clip space coordinates from range: [-w, w] to [-1, 1]. */
    vec3 projCoords = pos.xyz/pos.w;

    /* the depth buffer is using coordinates in the range: [0, 1] */
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture2D(texSamplerShadows, projCoords.xy).r;

    float currentDepth = projCoords.z;
    float bias = 0.001;
    return currentDepth - bias < closestDepth ? 1 : 0;
}

vec3 diffuse(vec3 lightPos,
            vec4 fragPosLightSpaceUp, vec4 fragPosLightSpaceLeft, vec4 fragPosLightSpaceDown, vec4 fragPosLightSpaceRight,
            sampler2D texDarkUp, sampler2D texDarkLeft, sampler2D texDarkDown, sampler2D texDarkRight) {

    /* Check to see if light will hit the fragment from the ball light source */
    /* first select which shadow map to use for this operation */
    vec3 lightToFrag = fragPosition - lightPos;
    vec3 lightDirection = normalize(lightToFrag);
    int inLight = 0;

    if ((lightDirection.x >= 0.0 && lightDirection.y >= 0.0 && lightDirection.x <= lightDirection.y) ||
        (lightDirection.x <= 0.0 && lightDirection.y >= 0.0 && -lightDirection.x <= lightDirection.y)) {
        /* up shadow map */
        inLight = ShadowCalculation(fragPosLightSpaceUp, texDarkUp);
    } else if ((lightDirection.x <= 0.0 && lightDirection.y <= 0.0 && lightDirection.x <= lightDirection.y) ||
             (lightDirection.x <= 0.0 && lightDirection.y >= 0.0 && -lightDirection.x >= lightDirection.y)) {
        /* left shadow map */
        inLight = ShadowCalculation(fragPosLightSpaceLeft, texDarkLeft);
    } else if ((lightDirection.x >= 0.0 && lightDirection.y <= 0.0 && lightDirection.x <= -lightDirection.y) ||
               (lightDirection.x <= 0.0 && lightDirection.y <= 0.0 && lightDirection.x >= lightDirection.y)) {
        /* down shadow map */
        inLight = ShadowCalculation(fragPosLightSpaceDown, texDarkDown);
    } else {
        /* ((lightDirection.x >= 0.0 && lightDirection.y >= 0.0 && lightDirection.x >= lightDirection.y) ||
            (lightDirection.x >= 0.0 && lightDirection.y <= 0.0 && lightDirection.x >= -lightDirection.y)) */
        /* right shadow map */
        inLight = ShadowCalculation(fragPosLightSpaceRight, texDarkRight);
    }

    float diff = 0.02;
    if (inLight == 1) {
        diff = max(dot(fragNormal, -lightDirection), 0.0);
    }

    float rSquaredMultiplier = 4.0;
    float smallValue = 0.00001;

    float rSquared = max(dot(lightToFrag, lightToFrag), smallValue);
    vec3 diffuse = vec3(diff/(rSquaredMultiplier * rSquared));

    return diffuse;
}

void main() {
    vec3 diffuse1 = diffuse(lightPos1, fragPosLightSpace1Up, fragPosLightSpace1Left,
        fragPosLightSpace1Down, fragPosLightSpace1Right,
        texDark1Up, texDark1Left, texDark1Down, texDark1Right);

    gl_FragColor = vec4(diffuse1, 1.0) * vec4(fragColor, 1.0);
}
