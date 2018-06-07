#version 430 core

uniform mat3 matrix;

out layout(location = 0) vec2 pos;

const vec2 vertices[4] = vec2[](
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0,  -1.0),
    vec2( 1.0, 1.0)
);

void main(void) {
    const vec2 vertexPos = vertices[gl_VertexID];
    pos = vec3(matrix * vec3(vertexPos, 1.0)).xy;
    gl_Position = vec4(vertexPos, 0.0, 1.0);
}
 
