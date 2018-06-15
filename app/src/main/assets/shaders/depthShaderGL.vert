#version 100
precision mediump float;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

attribute vec3 inPosition;
attribute vec3 inColor;
attribute vec2 inTexCoord;
attribute vec3 inNormal;

varying vec3 fragColor;

void main() {
    fragColor = inColor;
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
}
