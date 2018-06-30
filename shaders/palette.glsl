#define PALETTE_INDEX(samplerType, valueType)\
valueType paletteIndex(samplerType palette, const Index index) {\
    return texelFetch(palette, ivec2(index, 0));\
}
PALETTE_INDEX(sampler2DRect, vec4)
PALETTE_INDEX(usampler2DRect, uvec4)
PALETTE_INDEX(isampler2DRect, ivec4)

// Passing images to functions doesn't work?
//#define PALETTE_IMAGE(imageType, valueType)\
//valueType paletteImage(imageType palette, const Index index) {\
//    return imageLoad(palette, ivec2(index, 0));\
//}
//PALETTE_IMAGE(image2DRect, vec4)
//PALETTE_IMAGE(uimage2DRect, uvec4)
//PALETTE_IMAGE(iimage2DRect, ivec4)

#define PALETTE_SIZE(samplerType)\
Index paletteSize(samplerType palette) {\
    return Index(textureSize(palette).x);\
}
PALETTE_SIZE(sampler2DRect)
PALETTE_SIZE(usampler2DRect)
PALETTE_SIZE(isampler2DRect)

Index quantiseLut(usampler3D lut, const vec3 rgb) {
    const ivec3 size = textureSize(lut, 0);
    const ivec3 coord = clamp(ivec3(floor(clamp(rgb, 0.0, 1.0) * size)), ivec3(0), size - 1);
    const Index index = texelFetch(lut, coord, 0).x;
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
    for (Index index = 0; index < paletteSize(palette); ++index) {\
        if (index != transparentIndex) {\
            const vec4 paletteColour = vec4(toUnit(paletteIndex(palette, index), componentScale));\
            const float colourDistance = distance(rgba, paletteColour);\
            if (colourDistance < nearestDistance) {\
                nearestIndex = index;\
                nearestDistance = colourDistance;\
            }\
        }\
    }\
    return nearestIndex;\
}
QUANTISE_BRUTE_FORCE(sampler2DRect)
QUANTISE_BRUTE_FORCE(usampler2DRect)
QUANTISE_BRUTE_FORCE(isampler2DRect)

#define QUANTISE_BRUTE_FORCE_ALPHA_THRESHOLD(samplerType)\
Index quantiseBruteForce(samplerType palette, const uint componentScale, const vec4 rgba, const float alphaThreshold, const Index transparentIndex) {\
    if (rgba.a <= alphaThreshold) return transparentIndex;\
    else return quantiseBruteForce(palette, componentScale, rgba, transparentIndex);\
}
QUANTISE_BRUTE_FORCE_ALPHA_THRESHOLD(sampler2DRect)
QUANTISE_BRUTE_FORCE_ALPHA_THRESHOLD(usampler2DRect)
QUANTISE_BRUTE_FORCE_ALPHA_THRESHOLD(isampler2DRect)
