#define QUANTISE(samplerType) \
uint quantise(samplerType palette, const uint componentScale, const vec4 colour) { \
    uint nearestIndex = 0; \
    float nearestDistance = pow(componentScale + 1, 3); \
    for (uint index = 0; index < uint(textureSize(palette, 0).x); ++index) { \
        const vec4 paletteColour = vec4(texelFetch(palette, ivec2(index, 0))) / float(componentScale); \
        const float colourDistance = distance(colour.rgb, paletteColour.rgb); \
        if (colourDistance < nearestDistance) { \
            nearestIndex = index; \
            nearestDistance = colourDistance; \
        } \
    } \
    return nearestIndex; \
}

QUANTISE(sampler2DRect)

QUANTISE(usampler2DRect)

QUANTISE(isampler2DRect)
