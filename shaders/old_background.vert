#version 430 core

uniform mat3 inverseViewportTransform;

layout(location = 0) in vec2 vertexPos;

out vec2 viewportPos;

void main(void) {
    viewportPos = vec3(inverseViewportTransform * vec3(vertexPos, 1.0)).xy;
    gl_Position = vec4(vertexPos, 0.0, 1.0);
}
 
