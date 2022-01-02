#include "types.glsl"
#include "util.glsl"

// Common parts
struct Point {
    vec2 pos;
    float pressure;
    vec4 quaternion;
    float age;
    float distance;
};
layout(std430, binding = 0) buffer StorageData
{
    Point points[];
};

struct Dab {
    float hardness;
    float opacity;
};
layout(std140, binding = 0) uniform UniformData {
    uniform mat4 object;
    uniform mat4 worldToBuffer;
    uniform mat4 bufferToClip;
    Colour colour;
    Dab dab;
};

/////////////////////////
#if defined(VERTEX_STAGE)
/////////////////////////

void main(void) {
    Point point = points[gl_InstanceID];
    gl_Position = bufferToClip * (worldToBuffer * vec4(point.pos, 0.0, 1.0));
}

/////////////////////////////
#elif defined(GEOMETRY_STAGE)
/////////////////////////////

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 pos;

void main(void)
{
    for (uint i = 0u; i < 4u; ++i) {
        pos = vertices[i];
        vec4 delta = bufferToClip * vec4(pos * 32.0, 0.0, 1.0) - bufferToClip * vec4(0.0, 0.0, 0.0, 1.0);
        gl_Position = gl_in[0].gl_Position + vec4(delta.xy, 0.0, 0.0);
        EmitVertex();
    }
    EndPrimitive();
}

/////////////////////////////
#elif defined(FRAGMENT_STAGE)
/////////////////////////////

#include "distance.glsl"

in vec2 pos;

#if defined(BRUSH_DISTANCE)
float dabDistance(const vec2 pos) {
    return DISTANCE(pos);
}

Colour dabColour(const vec2 pos) {
    float weight;
    weight = clamp(1.0 - dabDistance(pos), 0.0, 1.0);
    weight = clamp(weight * (1.0 / (1.0 - dab.hardness)), 0.0, 1.0);
    weight *= dab.opacity;
    float alpha = colour.rgba.a * weight;
    uint index = INDEX_INVALID;
    if (alpha == colour.rgba.a) {
        index = colour.index;
    }
    return Colour(vec4(colour.rgba.rgb, alpha), INDEX_INVALID);
}
#elif defined(BRUSH_BUFFER)
float dabDistance(const vec2 pos) {
    return distanceChebyshev(pos);
}

Colour dabColour(const vec2 pos) {
    return COLOUR_INVALID;
}
#endif //BRUSH

Colour src(void) {
    Colour colour = dabColour(pos);
    gl_FragDepth = pow(dabDistance(pos), 2.0);
//    if (gl_FragDepth >= 1.0) discard;
    return colour;
}

#define FRAGMENT_MAIN
#include "fragment.glsl"

//////////////
#endif //STAGE
//////////////
