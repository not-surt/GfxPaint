#if !defined(BUFFER_GLSL)
#define BUFFER_GLSL

#include "types.glsl"
#include "util.glsl"

#define BUFFER_RGBA(NAME, LOCATION, SAMPLER_TYPE, FORMAT_SCALE, SCALAR_VALUE_TYPE)\
uniform layout(location = LOCATION) SAMPLER_TYPE NAME##Texture;\
\
Colour NAME(const vec2 pos) {\
    return toUnit(texelFetch(NAME##Texture, ivec2(floor(pos)), 0), SCALAR_VALUE_TYPE(FORMAT_SCALE));\
}

#define BUFFER_INDEXED(NAME, LOCATION, SAMPLER_TYPE, FORMAT_SCALE, SCALAR_VALUE_TYPE)\
uniform layout(location = LOCATION) SAMPLER_TYPE NAME##Texture;\
\
Colour NAME(const vec2 pos) {\
    uint index = texelFetch(NAME##Texture, ivec2(floor(pos)), 0).x;\
    return NAME##Palette(index);\
}

#endif // BUFFER_GLSL
