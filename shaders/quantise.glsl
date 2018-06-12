Index quantiseLut(usampler3D lut, const vec3 colour) {\
    const ivec3 size = textureSize(lut, 0);\
    const ivec3 coord = ivec3(clamp(colour, 0.0, 1.0) * size);\
    const Index index = texelFetch(lut, coord, 0).x;\
    return index;
}

#define QUANTISE_BRUTE_FORCE(samplerType)\
Index quantiseBruteForce(samplerType palette, const uint componentScale, const vec4 rgba, const float alphaThreshold, const Index destTransparent) {\
    if (destTransparent != INDEX_INVALID && rgba.a <= alphaThreshold) return destTransparent;\
    else {\
        Index nearestIndex = 0;\
        float nearestDistance = INF;\
        for (Index index = 0; index < Index(textureSize(palette).x); ++index) {\
            if (index != destTransparent) {\
                const vec4 paletteColour = vec4(toUnit(texelFetch(palette, ivec2(index, 0)), componentScale));\
                const float colourDistance = distance(rgba, paletteColour);\
                if (colourDistance < nearestDistance) {\
                    nearestIndex = index;\
                    nearestDistance = colourDistance;\
                }\
            }\
        }\
        return nearestIndex;\
    }\
}
QUANTISE_BRUTE_FORCE(sampler2DRect)
QUANTISE_BRUTE_FORCE(usampler2DRect)
QUANTISE_BRUTE_FORCE(isampler2DRect)
