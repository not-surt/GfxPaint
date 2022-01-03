#if !defined(PALETTE_GLSL)
#define PALETTE_GLSL

#define PALETTE_INDEX(samplerType, valueType)\
valueType paletteIndex(samplerType palette, const Index index) {\
    return texelFetch(palette, ivec2(index, 0), 0);\
}
PALETTE_INDEX(sampler2D, vec4)
PALETTE_INDEX(usampler2D, uvec4)
PALETTE_INDEX(isampler2D, ivec4)

#define PALETTE_SIZE(samplerType)\
Index paletteSize(samplerType palette) {\
    return Index(textureSize(palette, 0).x);\
}
PALETTE_SIZE(sampler2D)
PALETTE_SIZE(usampler2D)
PALETTE_SIZE(isampler2D)

Index quantiseLut(usampler3D lut, const vec3 rgb) {
    ivec3 size = textureSize(lut, 0);
    ivec3 coord = clamp(ivec3(floor(clamp(rgb, vec3(0.0), vec3(1.0)) * vec3(size))), ivec3(0), size - 1);
    Index index = texelFetch(lut, coord, 0).x;
    return index;
}

Index quantiseLut(usampler3D lut, const vec4 rgba, const float alphaThreshold, const Index transparentIndex) {
    if (rgba.a <= alphaThreshold) return transparentIndex;
    else return quantiseLut(lut, rgba.rgb);
}

#define QUANTISE_BRUTE_FORCE(samplerType)\
Index quantiseBruteForce(samplerType palette, const uint componentScale, const vec4 rgba, const Index transparentIndex) {\
    Index nearestIndex = INDEX_INVALID;\
    float nearestDistance = FLOAT_INF;\
    for (Index index = 0u; index < paletteSize(palette); ++index) {\
        if (index != transparentIndex) {\
            vec4 paletteColour = vec4(toUnit(vec4(paletteIndex(palette, index)), float(componentScale)));\
            float colourDistance = distance(rgba, paletteColour);\
            if (colourDistance < nearestDistance) {\
                nearestIndex = index;\
                nearestDistance = colourDistance;\
            }\
        }\
    }\
    return nearestIndex;\
}
QUANTISE_BRUTE_FORCE(sampler2D)
QUANTISE_BRUTE_FORCE(usampler2D)
QUANTISE_BRUTE_FORCE(isampler2D)

#define QUANTISE_BRUTE_FORCE_ALPHA_THRESHOLD(samplerType)\
Index quantiseBruteForce(samplerType palette, const uint componentScale, const vec4 rgba, const float alphaThreshold, const Index transparentIndex) {\
    if (rgba.a <= alphaThreshold) return transparentIndex;\
    else return quantiseBruteForce(palette, componentScale, rgba, transparentIndex);\
}
QUANTISE_BRUTE_FORCE_ALPHA_THRESHOLD(sampler2D)
QUANTISE_BRUTE_FORCE_ALPHA_THRESHOLD(usampler2D)
QUANTISE_BRUTE_FORCE_ALPHA_THRESHOLD(isampler2D)

#endif // PALETTE_GLSL
