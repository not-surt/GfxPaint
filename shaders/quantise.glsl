Index quantiseLut(usampler3D lut, const vec3 rgb) {
    const ivec3 size = textureSize(lut, 0);
    const ivec3 coord = clamp(ivec3(floor(clamp(rgb, 0.0, 1.0) * size)), ivec3(0), size - 1);
    const Index index = texelFetch(lut, coord, 0).x;
    return index;
}

Index quantiseLut(usampler3D lut, const vec4 rgba, const float alphaThreshold, const Index transparentIndex) {
    if (transparentIndex != INDEX_INVALID && rgba.a <= alphaThreshold) return transparentIndex;
    else return quantiseLut(lut, rgba.rgb);
}

#define QUANTISE_BRUTE_FORCE(samplerType)\
Index quantiseBruteForce(samplerType palette, const uint componentScale, const vec4 rgba, const float alphaThreshold, const Index transparentIndex) {\
    if (transparentIndex != INDEX_INVALID && rgba.a <= alphaThreshold) return transparentIndex;\
    else {\
        Index nearestIndex = 0;\
        float nearestDistance = INF;\
        for (Index index = 0; index < Index(textureSize(palette).x); ++index) {\
            if (index != transparentIndex) {\
                const vec4 paletteColour = vec4(toUnit(texelFetch(palette, ivec2(index, 0)), componentScale));\
                float colourDistance;\
                if (transparentIndex != INDEX_INVALID) {\
                    colourDistance = distance(vec4(rgba.rgb, 1.0), vec4(paletteColour.rgb, 1.0));\
                }\
                else {\
                    colourDistance = distance(rgba, paletteColour);\
                }\
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
