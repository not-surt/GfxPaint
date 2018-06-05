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
