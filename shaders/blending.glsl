// Scalar blend functions

float blendNormalScalar(const float dest, const float src) {
    return src;
}

float blendMultiplyScalar(const float dest, const float src) {
    return dest * src;
}

float blendScreenScalar(const float dest, const float src) {
    return dest + src - (dest * src);
}

float blendHardLightScalar(const float dest, const float src);///////////////////
float blendOverlayScalar(const float dest, const float src) {
    return blendHardLightScalar(src, dest);
}

float blendDarkenScalar(const float dest, const float src) {
    return min(dest, src);
}

float blendLightenScalar(const float dest, const float src) {
    return max(dest, src);
}

float blendColourDodgeScalar(const float dest, const float src) {
    return (dest == 0.0) ? 0.0
        : (src == 1.0) ? 1.0
        : min(1.0, dest / (1.0 - src));
}

float blendColourBurnScalar(const float dest, const float src) {
    return (dest == 1.0) ? 1.0
        : (src == 0.0) ? 0.0
        : 1.0 - min(1.0, (1.0 - dest) / src);
}

float blendHardLightScalar(const float dest, const float src) {
    return (src <= 0.5) ? blendMultiplyScalar(dest, 2.0 * src)
        : blendScreenScalar(dest, 2.0 * src - 1.0);
}

float blendSoftLightScalar(const float dest, const float src) {
    float d = (dest <= 0.25) ? ((16.0 * dest - 12.0) * dest + 4.0) * dest
        : sqrt(dest);
    return (src <= 0.5) ? dest - (1.0 - 2.0 * src) * dest * (1.0 - dest)
        : dest + (2.0 * src - 1.0) * (d - dest);
}

float blendDifferenceScalar(const float dest, const float src) {
    return abs(dest - src);
}

float blendExclusionScalar(const float dest, const float src) {
    return dest + src - 2.0 * dest * src;
}



// Seperable vector blend functions

#define SCALAR_BLEND_VEC3(blend, dest, src) vec3(blend(dest.r, src.r), blend(dest.g, src.g), blend(dest.b, src.b))

vec3 blendNormal(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendNormalScalar, dest, src);
}

vec3 blendMultiply(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendMultiplyScalar, dest, src);
}

vec3 blendScreen(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendScreenScalar, dest, src);
}

vec3 blendOverlay(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendOverlayScalar, dest, src);
}

vec3 blendDarken(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendDarkenScalar, dest, src);
}

vec3 blendLighten(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendLightenScalar, dest, src);
}

vec3 blendColourDodge(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendColourDodgeScalar, dest, src);
}

vec3 blendColourBurn(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendColourBurnScalar, dest, src);
}

vec3 blendHardLight(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendHardLightScalar, dest, src);
}

vec3 blendSoftLight(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendSoftLightScalar, dest, src);
}

vec3 blendDifference(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendDifferenceScalar, dest, src);
}

vec3 blendExclusion(const vec3 dest, const vec3 src) {
    return SCALAR_BLEND_VEC3(blendExclusionScalar, dest, src);
}



// Non-seperable vector blend functions

float lum(const vec3 colour) {
    return 0.3 * colour.r + 0.59 * colour.g + 0.11 * colour.b;
}

float _clipColourLower(const float l, const float n, const float component) {
    return l + (((component - l) * l) / (l - n));
}

float _clipColourUpper(const float l, const float x, const float component) {
    return l + (((component - l) * (1.0 - l)) / (x - l));
}

vec3 clipColour(const vec3 colour) {
    float l = lum(colour);
    float n = min(min(colour.r, colour.g), colour.b);
    float x = max(max(colour.r, colour.g), colour.b);
    return (n < 0.0)
        ? vec3(_clipColourLower(l, n, colour.r), _clipColourLower(l, n, colour.g), _clipColourLower(l, n, colour.b))
        : vec3(_clipColourUpper(l, x, colour.r), _clipColourUpper(l, x, colour.g), _clipColourUpper(l, x, colour.b));
}

vec3 setLum(const vec3 colour, const float l) {
    float d = l - lum(colour);
    return clipColour(colour + vec3(d));
}

float sat(const vec3 colour) {
    return max(max(colour.r, colour.g), colour.b) - min(min(colour.r, colour.g), colour.b);
}

vec3 setSat(vec3 colour, const float s) {
    int componentMin = (colour[0] <= colour[1] && colour[0] <= colour[2]) ? 0
        : (colour[1] <= colour[0] && colour[1] <= colour[2]) ? 1 : 2;
    int componentMax = (colour[0] >= colour[1] && colour[0] >= colour[2]) ? 0
        : (colour[1] >= colour[0] && colour[1] >= colour[2]) ? 1 : 2;
    int componentMid = ((colour[1] <= colour[0] && colour[0] <= colour[2]) || (colour[2] <= colour[0] && colour[0] <= colour[1])) ? 0
        : ((colour[0] <= colour[1] && colour[1] <= colour[2]) || (colour[2] <= colour[1] && colour[1] <= colour[0])) ? 1 : 2;

    if (colour[componentMax] > colour[componentMin]) {
        colour[componentMid] = ((colour[componentMid] - colour[componentMin]) * s) / (colour[componentMax] - colour[componentMin]);
        colour[componentMax] = s;
    }
    else {
        colour[componentMid] = colour[componentMax] = 0.0;
    }
    colour[componentMin] = 0.0;

    return colour;
}

vec3 blendHue(const vec3 dest, const vec3 src) {
    return setLum(setSat(src, sat(dest)), lum(dest));
}

vec3 blendSaturation(const vec3 dest, const vec3 src) {
    return setLum(setSat(dest, sat(src)), lum(dest));
}

vec3 blendColour(const vec3 dest, const vec3 src) {
    return setLum(src, lum(dest));
}

vec3 blendLuminosity(const vec3 dest, const vec3 src) {
    return setLum(dest, lum(src));
}
