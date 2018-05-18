#version 430 core

const float base = 7.0 / 16.0;
const float offset = 1.0 / 16.0;
const float light = base + offset;
const float dark = base - offset;

uniform float checkerSize;

in vec2 viewportPos;

out vec4 fragment;

void main(void) {
    vec4 colour = !((mod(viewportPos.x, checkerSize * 2) >= checkerSize) == (mod(viewportPos.y, checkerSize * 2) >= checkerSize))
                  ? vec4(vec3(light), 1.0)
                  : vec4(vec3(dark), 1.0);
    fragment = colour;
}
