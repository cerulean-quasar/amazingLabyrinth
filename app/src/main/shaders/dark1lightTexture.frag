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
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in vec4 fragPosLightSpace1Up;
layout(location = 5) in vec4 fragPosLightSpace1Left;
layout(location = 6) in vec4 fragPosLightSpace1Down;
layout(location = 7) in vec4 fragPosLightSpace1Right;

layout(binding = 2) uniform sampler2D texSampler;
layout(set = 0, binding = 3) uniform UniformBufferObject {
    vec3 pos1;
} light;

/* shadow maps for the first light, first one is for the up direction, then circle around
 * counterclockwise assigning numbers
 */
layout(binding = 4) uniform sampler2D texDark1Up;
layout(binding = 5) uniform sampler2D texDark1Left;
layout(binding = 6) uniform sampler2D texDark1Down;
layout(binding = 7) uniform sampler2D texDark1Right;

layout(location = 0) out vec4 outColor;

int ShadowCalculation(vec4 pos, sampler2D texSamplerShadows) {
    /* perspective divide: transform clip space coordinates from range: [-w, w] to [-1, 1]. */
    vec3 projCoords = pos.xyz/pos.w;

    /* the depth buffer is using coordinates in the range: [0, 1] */
    projCoords = vec3(projCoords.x * 0.5 + 0.5, 0.5 + projCoords.y * 0.5, projCoords.z);
    float closestDepth = texture(texSamplerShadows, projCoords.xy).r;

    float currentDepth = projCoords.z;
    float bias = 0.001;
    return currentDepth - bias < closestDepth ? 1 : 0;
}

vec3 diffuse(vec3 lightPos,
            vec4 fragPosLightSpaceUp, vec4 fragPosLightSpaceLeft, vec4 fragPosLightSpaceDown, vec4 fragPosLightSpaceRight,
            sampler2D texDarkUp, sampler2D texDarkLeft, sampler2D texDarkDown, sampler2D texDarkRight) {
    float smallValue = 0.01;

    /* Check to see if light will hit the fragment from the ball light source */
    /* first select which shadow map to use for this operation */
    vec3 lightToFrag = fragPosition - lightPos;
    vec3 lightDirection = normalize(lightToFrag);
    int inLight = 0;
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
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
            (lightDirection.x >= 0.0 && lightDirection.y <= 0.0 && lightDirection.x >= -lightDirection.y))
        */
        /* right shadow map */
        inLight = ShadowCalculation(fragPosLightSpaceRight, texDarkRight);
    }

    if (inLight == 1) {
        float diff = max(dot(fragNormal, lightDirection), 0.0);
        float rSquared = lightToFrag.x*lightToFrag.x + lightToFrag.y*lightToFrag.y + lightToFrag.z*lightToFrag.z;
        //float rSquared = lightToFrag.x + lightToFrag.y + lightToFrag.z;
        rSquared = rSquared * 100.0;
        if (rSquared < smallValue) {
            rSquared = smallValue;
        }
        diffuse = diff/rSquared * vec3(1.0, 1.0, 1.0);
    }

    return diffuse;
}

void main() {
    vec3 diffuse1 = diffuse(light.pos1, fragPosLightSpace1Up, fragPosLightSpace1Left,
        fragPosLightSpace1Down, fragPosLightSpace1Right,
        texDark1Up, texDark1Left, texDark1Down, texDark1Right);

    outColor = vec4(diffuse1, 1.0) * texture(texSampler, fragTexCoord);
}
