#version 430 core

uniform mat3 viewportMatrix;
uniform mat3 matrix;

// Source
uniform ivec2 srcRectPos;
uniform ivec2 srcRectSize;
layout(location = 0) uniform usampler2DRect srcTexture;

layout(location = 0) in vec2 vertexPos;

layout(location = 0) out vec2 texelPos;

void main(void) {
    texelPos = vec2(srcRectPos) + vertexPos * vec2(srcRectSize);
    gl_Position = vec4(matrix * vec3(vertexPos * vec2(srcRectSize), 1.0), 1.0);
}
