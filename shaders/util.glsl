float scaleShift(const float scale, const float shift, const float value) {
    return scale * value + shift;
}

float bias(const float biasVal, const float value) {
    return pow(value, log(biasVal) / log(0.5));
}

float gain(const float gainVal, const float value) {
    return value <= 0.5 ? bias(2.0 * value, 1.0 - gainVal) / 2.0 : 1.0 - bias(2.0 - 2.0 * value, 1.0 - gainVal) / 2.0;
}

float rand(const vec2 seed){
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 premultiply(vec4 colour) {
    return vec4(colour.rgb * colour.a, colour.a);
}

vec4 unpremultiply(const vec4 colour) {
    if (colour.a == 0.0) return colour;
    else return vec4(colour.rgb / colour.a, colour.a);
}

#define FROM_UNIT_FLOAT(unitType, valueType, scaleType)\
valueType fromUnit(const unitType value, const scaleType scale) {\
    return clamp(value * scale, 0.0, scale);\
}
FROM_UNIT_FLOAT(float, float, float)
FROM_UNIT_FLOAT(vec4, vec4, float)

#define TO_UNIT_FLOAT(unitType, valueType, scaleType)\
unitType toUnit(const valueType value, const scaleType scale) {\
    return clamp(value / scale, 0.0, 1.0);\
}
TO_UNIT_FLOAT(float, float, float)
TO_UNIT_FLOAT(vec4, vec4, float)

#define FROM_UNIT_INT(unitType, valueType, scaleType)\
valueType fromUnit(const unitType value, const scaleType scale) {\
    return clamp(valueType(value * scale), 0, scale);\
}
FROM_UNIT_INT(float, int, int)
FROM_UNIT_INT(vec4, ivec4, int)
FROM_UNIT_INT(float, uint, uint)
FROM_UNIT_INT(vec4, uvec4, uint)

#define TO_UNIT_INT(unitType, valueType, scaleType)\
unitType toUnit(const valueType value, const scaleType scale) {\
    return clamp(unitType(value) / unitType(scale), 0.0, 1.0);\
}
TO_UNIT_INT(float, int, int)
TO_UNIT_INT(vec4, ivec4, int)
TO_UNIT_INT(float, uint, uint)
TO_UNIT_INT(vec4, uvec4, uint)

#define PALETTE_SAMPLE(samplerType, valueType)\
valueType paletteSample(samplerType palette, const Index index) {\
    return texelFetch(palette, ivec2(index, 0));\
}
PALETTE_SAMPLE(sampler2DRect, vec4)
PALETTE_SAMPLE(usampler2DRect, uvec4)
PALETTE_SAMPLE(isampler2DRect, ivec4)

float stairstep(const float value, const float size) {
    return round(value / size) * size;
}

float snap(const float offset, const float size, const float target, const bool relative, const float relativeTo) {
    const float shift = (relative ? relativeTo : offset);
    return size != 0.0 ? stairstep(target - shift, size) + shift : target;
}

vec2 snap2d(const vec2 offset, const vec2 size, const vec2 target, const bool relative, const vec2 relativeTo) {
    return vec2(snap(offset.x, size.x, target.x, relative, relativeTo.x),
                snap(offset.y, size.y, target.y, relative, relativeTo.y));
}

#define PIXEL_SNAP_OFF 0
#define PIXEL_SNAP_CENTRE 1
#define PIXEL_SNAP_EDGE 2
#define PIXEL_SNAP_BOTH 3
#define PIXEL_SNAP_AUTO 4

float pixelSnapOffset(const int pixelSnap, const float target, const float size) {
    switch (pixelSnap) {
    case PIXEL_SNAP_CENTRE: return 0.5f;
    case PIXEL_SNAP_EDGE: return 0.0f;
    case PIXEL_SNAP_BOTH: return abs(target - floor(target) - 0.5f) < 0.25f ? 0.5f : 1.0f;
    case PIXEL_SNAP_AUTO: return int(round(size)) % 2 == 0 ? 0.0f : 0.5f;
    default: return target;
    }
}

vec2 pixelSnap(const ivec2 pixelSnap, const vec2 target, const vec2 size) {
    const float offsetX = pixelSnapOffset(pixelSnap.x, target.x, size.x);
    const float offsetY = pixelSnapOffset(pixelSnap.y, target.y, size.y);
    return snap2d(vec2(offsetX, offsetY), vec2(1.0, 1.0), target, false, vec2(0.0, 0.0));
}
