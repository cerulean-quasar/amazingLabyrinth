#version 100
precision mediump float;

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

    /* the depth buffer is using coordinates in the range: [0, 1] */
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }
    float closestDepth = texture2D(texShadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;
    float bias = 0.005;
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

    gl_FragColor = vec4(fragColor + diffuse*(1.0 - shadow), 1.0) * texture2D(texSampler, fragTexCoord);
}
