#version 430 core

// Buffer formats
const int FormatDefault = 0; // For default framebuffer, output only, no blending
const int FormatIndexed = 1;
const int FormatRgba = 2;

// Blend modes
const int BlendModeMix = 0;
const int BlendModeReplace = 1;
const int BlendModeErase = 2;

// Source
uniform uvec2 srcRectPos;
uniform uvec2 srcRectSize;
uniform int srcFormat;
layout(location = 0) uniform usampler2DRect srcTexture;
uniform bool srcHasPalette;
layout(location = 1) uniform usampler2DRect srcPalette;
uniform bool srcHasTransparency;
uniform int srcTransparentIndex;
uniform uvec4 srcTransparentColour;

// Destination
uniform uvec2 destRectPos;
uniform uvec2 destRectSize;
uniform int destFormat;
layout(location = 2) uniform usampler2DRect destTexture;
uniform bool destHasPalette;
layout(location = 3) uniform usampler2DRect destPalette;
uniform bool destHasTransparency;
uniform int destTransparentIndex;
uniform uvec4 destTransparentColour;

uniform int blendMode;

layout(location = 0) in vec2 texelPos;

layout(location = 0) out vec4 fragmentDefault;
layout(location = 1) out uint fragmentIndexed;
layout(location = 2) out uvec4 fragmentRgba;

vec4 premultiply(const vec4 rgba) {
    if (rgba.a == 0.0) return rgba;
    else return vec4(rgba.rgb * rgba.a, rgba.a);
}

vec4 unpremultiply(const vec4 rgba) {
    if (rgba.a == 0.0) return rgba;
    else return vec4(rgba.rgb / rgba.a, rgba.a);
}

void main(void) {
    vec4 srcColour;
    bool blendIndexed = false;

    if (srcFormat == FormatIndexed && destFormat == FormatIndexed) {
        const uint srcIndex = texelFetch(srcTexture, ivec2(floor(texelPos))).r;
        if (srcHasTransparency && srcIndex == srcTransparentIndex) discard;
        else fragmentIndexed = srcIndex;
    }
    else {
        switch (srcFormat) {
        case FormatIndexed: {
            const uint srcIndex = texelFetch(srcTexture, ivec2(floor(texelPos))).r;
            if (srcHasTransparency && srcIndex == srcTransparentIndex) discard;
            else {
                if (srcHasPalette) srcColour = vec4(texelFetch(srcPalette, ivec2(srcIndex, 0))) / 255;
                else srcColour = vec4(vec3(srcIndex), 255) / 255;
            }
        } break;
        case FormatRgba: {
            const uvec4 srcRgba = texelFetch(srcTexture, ivec2(floor(texelPos)));
            srcColour = vec4(srcRgba) / 255;
        } break;
        }

        if (destFormat == FormatDefault) {
            fragmentDefault = vec4(srcColour.rgb * srcColour.a, srcColour.a);
        }
        else {
            vec4 destColour = texelFetch(destTexture, ivec2(floor(gl_FragCoord)));

            switch (destFormat) {
            case FormatIndexed: {
                const uint destIndex = texelFetch(destTexture, ivec2(floor(gl_FragCoord))).r;
                int nearestIndex = -1;
                float nearestDistance = 0.0;
                for (int index = 0; index < textureSize(destPalette).x; ++index) {
                    const float distance = length((vec4(texelFetch(destPalette, ivec2(index, 0))) / 255) - srcColour);
                    if (nearestIndex < 0 || distance < nearestDistance) {
                        nearestIndex = index;
                        nearestDistance = distance;
                    }
                }
                if (nearestIndex < 0) discard;
                else fragmentIndexed = nearestIndex;
            } break;
            case FormatRgba: {
                const uvec4 destRgba = texelFetch(destTexture, ivec2(floor(gl_FragCoord)));
                fragmentRgba = uvec4(floor(vec4(srcColour.rgb * srcColour.a, srcColour.a) * 255));
            } break;
            }
        }

        /*switch (blendMode) {
        case BlendModeMix: {
            if (blendIndexed) {
                fragmentIndexed = (srcIndex != transparentIndex ? srcIndex : destIndex);
            }
            else {
                if (srcRgba != transparentRgba) {
                    const float a = srcRgba.a + destRgba.a * (1.0 - srcRgba.a);
                    const float rgb = srcRgba.rgb + destRgba.rgb * (1.0 - srcRgba.a);
                    fragmentRgba = vec4(rgb, a);
                }
                else {
                    fragmentRgba = destRgba;
                }

            }
        } break;
        case BlendModeReplace: {
            fragmentRgba = srcRgba;
        } break;
        case BlendModeErase: {
        } break;
        }*/
//        vec4 blend(vec4 src, vec4 dest) {
//            vec3 rgb;
//            float a;
//            #if defined(Mix)
//                a = src.a + dest.a * (1.0 - src.a);
//                rgb = (src.rgb * src.a + dest.rgb * dest.a * (1.0 - src.a)) / a;
//            #elif defined(Erase)
//                rgb = dest.rgb;
//                a = dest.a - src.a;
//            #elif defined(Add) || defined(Subtract)
//                #if defined(Subtract)
//                    src.a *= -1.0;
//                #endif
//                //rgb = src.rgb * (1.0 - dest.a) + (src.rgb * src.a + dest.rgb) * dest.a;
//                //a = src.a * (1.0 - dest.a) + dest.a;
//                rgb = src.rgb * src.a + dest.rgb;
//                a = dest.a;
//            #elif defined(Multiply) || defined(Divide)
//                #if defined(Divide)
//                    src.a = 1.0 / src.a;
//                #endif
//                rgb = src.rgb * src.a * dest.rgb;
//                a = dest.a;
//            #endif
//            #if defined(PreserveAlpha)
//                return vec4(rgb, dest.a);
//            #else
//                return vec4(rgb, a);
//            #endif
    }
}
