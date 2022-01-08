#if !defined(BUFFER_GLSL)
#define BUFFER_GLSL

#include "types.glsl"
#include "util.glsl"

Colour samplerColour(sampler2D sampler, const vec2 pos) {
    return Colour(texelFetch(sampler, ivec2(floor(pos)), 0), INDEX_INVALID);
}
Index samplerIndex(sampler2D sampler, const vec2 pos) {
    return Index(floor(texelFetch(sampler, ivec2(floor(pos)), 0).x));
}

#define BUFFER_RGBA(NAME, LOCATION, SAMPLER_TYPE, TEXTURE_NAME, FORMAT_SCALE, SCALAR_VALUE_TYPE)\
uniform layout(location = LOCATION) SAMPLER_TYPE TEXTURE_NAME;\
\
Colour NAME(const vec2 pos) {\
    return Colour(toUnit(texelFetch(TEXTURE_NAME, ivec2(floor(pos)), 0), SCALAR_VALUE_TYPE(FORMAT_SCALE)), INDEX_INVALID);\
}

#define BUFFER_INDEXED(NAME, LOCATION, SAMPLER_TYPE, TEXTURE_NAME, PALETTE_NAME)\
uniform layout(location = LOCATION) SAMPLER_TYPE TEXTURE_NAME;\
\
Colour NAME(const vec2 pos) {\
    Index index = texelFetch(TEXTURE_NAME, ivec2(floor(pos)), 0).x;\
    return PALETTE_NAME(vec2(float(index) + 0.5, 0.5));\
}

#endif // BUFFER_GLSL
