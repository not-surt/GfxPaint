#version 430 core

uniform mat3 brush; // brush to world
uniform mat3 projection; // world to image
uniform vec2 chunkOffset;
uniform vec2 chunkSize;

uniform sampler2D image;
uniform vec2 subImagePos;
uniform vec2 subImageSize;

in vec2 pos;
in vec2 instanceOffset;

out vec2 texturePos;
out vec2 destPos;

void main(void) {
    texturePos = pos;
    destPos = (brush * vec3(pos + instanceOffset, 1.0) - vec3(chunkOffset, 0.0)).xy;
    gl_Position = vec4(projection * vec3(destPos, 1.0), 1.0);
    destPos = (gl_Position.xy + vec2(1.0, 1.0)) / 2.0 * chunkSize;
}
 
