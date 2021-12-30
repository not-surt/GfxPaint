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

vec2 perp(const vec2 vector) {
    return vec2(-vector.y, vector.x);
}

// aka perp dot product, 2d cross product, wedge product
float perpProd(const vec2 a, const vec2 b) {
    return dot(perp(a), b);
}

bool intersectRays(const vec2 as, const vec2 ad, const vec2 bs, const vec2 bd, out vec2 p, out float u, out float v)
{
    vec2 delta = bs - as;
    float det = bd.x * ad.y - bd.y * ad.x;
    if (det != 0.0) {
        u = (delta.y * bd.x - delta.x * bd.y) / det;
        v = (delta.y * ad.x - delta.x * ad.y) / det;
        p = as + ad * u;
        return true;
    }
    else return false;
}

// By Inigo Quilez from https://www.iquilezles.org/www/articles/ibilinear/ibilinear.htm
vec2 inverseBilinear(vec2 p, vec2 a, vec2 b, vec2 c, vec2 d)
{
    vec2 res = vec2(-1.0);

    vec2 e = b - a;
    vec2 f = d - a;
    vec2 g = a - b + c - d;
    vec2 h = p - a;

    float k2 = perpProd(g, f);
    float k1 = perpProd(e, f) + perpProd(h, g);
    float k0 = perpProd(h, e);

    // if edges are parallel, this is a linear equation
    if (abs(k2) < 0.001) {
        res = vec2((h.x * k1 + f.x * k0) / (e.x * k1 - g.x * k0), -k0 / k1);
    }
    // otherwise, it's a quadratic
    else {
        float w = k1 * k1 - 4.0 * k0 * k2;
        if (w < 0.0) return vec2(-1.0);
        w = sqrt(w);

        float ik2 = 0.5 / k2;
        float v = (-k1 - w) * ik2;
        float u = (h.x - f.x * v) / (e.x + g.x * v);

        if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0) {
           v = (-k1 + w) * ik2;
           u = (h.x - f.x * v) / (e.x + g.x * v);
        }
        res = vec2(u, v);
    }

    return res;
}

// 00 10
// 01 11
#define BILINEAR(valueType)\
valueType bilinear(const valueType value00, const valueType value01, const valueType value10, const valueType value11, const vec2 pos) {\
    valueType interp0 = mix(value00, value01, pos.y);\
    valueType interp1 = mix(value10, value11, pos.y);\
    return mix(interp0, interp1, pos.x);\
}
BILINEAR(float)
BILINEAR(vec2)
BILINEAR(vec3)
BILINEAR(vec4)

#define FROM_UNIT(unitType, valueType, scaleType)\
valueType fromUnit(const unitType value, const scaleType scale) {\
    return clamp(valueType(value * unitType(scale)), valueType(0), valueType(scale));\
}
FROM_UNIT(float, float, float)
FROM_UNIT(vec4, vec4, float)
FROM_UNIT(float, int, int)
FROM_UNIT(vec4, ivec4, int)
FROM_UNIT(float, uint, uint)
FROM_UNIT(vec4, uvec4, uint)

#define TO_UNIT(unitType, valueType, scaleType)\
unitType toUnit(const valueType value, const scaleType scale) {\
    return clamp(unitType(value) / unitType(scale), unitType(0.0), unitType(1.0));\
}
TO_UNIT(float, float, float)
TO_UNIT(vec4, vec4, float)
TO_UNIT(float, int, int)
TO_UNIT(vec4, ivec4, int)
TO_UNIT(float, uint, uint)
TO_UNIT(vec4, uvec4, uint)

#define PALETTE_SAMPLE(samplerType, valueType)\
valueType paletteSample(samplerType palette, const Index index) {\
    return valueType(texelFetch(palette, ivec2(index, 0), 0));\
}
PALETTE_SAMPLE(sampler2D, vec4)
PALETTE_SAMPLE(usampler2D, uvec4)
PALETTE_SAMPLE(isampler2D, ivec4)

#define STAIRSTEP(valueType)\
valueType stairstep(const valueType value, const valueType size) {\
    return floor(value / size) * size;\
}
STAIRSTEP(float)
STAIRSTEP(vec2)
STAIRSTEP(vec3)
STAIRSTEP(vec4)

//float snap(const float offset, const float size, const float target, const bool relative, const float relativeTo) {
//    float shift = (relative ? relativeTo : offset);
//    return size != 0.0 ? stairstep(target - shift, size) + shift : target;
//}
#define SNAP(valueType)\
valueType snap(const valueType offset, const valueType size, const valueType target) {\
    return stairstep(target - offset, size) + offset;\
}
SNAP(float)
SNAP(vec2)
SNAP(vec3)
SNAP(vec4)

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
    float offsetX = pixelSnapOffset(pixelSnap.x, target.x, size.x);
    float offsetY = pixelSnapOffset(pixelSnap.y, target.y, size.y);
    return snap(vec2(offsetX, offsetY), vec2(1.0, 1.0), target);
}
