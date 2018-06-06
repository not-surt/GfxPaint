#define QUANTISE(samplerType)\
uint quantise(samplerType palette, const uint componentScale, const vec4 colour) {\
    uint nearestIndex = 0;\
    float nearestDistance = 1.0 / 0.0;\
    for (uint index = 0; index < uint(textureSize(palette, 0).x); ++index) {\
        const vec4 paletteColour = vec4(toUnit(uvec4(texelFetch(palette, ivec2(index, 0))), componentScale));\
        const float colourDistance = distance(colour.rgb, paletteColour.rgb);\
        if (colourDistance < nearestDistance) {\
            nearestIndex = index;\
            nearestDistance = colourDistance;\
        }\
    }\
    return nearestIndex;\
}

QUANTISE(sampler2DRect)

QUANTISE(usampler2DRect)

QUANTISE(isampler2DRect)
