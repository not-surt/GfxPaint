#version 430 core

uniform sampler2D dest;
uniform float bias;
uniform float gain;
uniform vec4 colour;
uniform float blendStrength;

uniform sampler2D image;
uniform vec2 subImagePos;
uniform vec2 subImageSize;

in vec2 texturePos;
in vec2 destPos;

out vec4 fragment;

float scaleShift(float scale, float shift, float value) {
    return scale * value + shift;
}

float biasFunc(float biasVal, float value) {
    return pow(value, log(biasVal) / log(0.5));
}

float gainFunc(float gainVal, float value) {
    return value <= 0.5 ? biasFunc(2.0 * value, 1.0 - gainVal) / 2.0 : 1.0 - biasFunc(2.0 - 2.0 * value, 1.0 - gainVal) / 2.0;
}

float type(vec2 pos) {
    float ret;
    #if defined(Pixel)
        ret = 0.0;
    #elif defined(Ellipse)
        ret = min(length(pos), 1.0);
    #elif defined(Rectangle)
        ret = min(max(abs(pos.x), abs(pos.y)), 1.0);
    #endif
    return ret;
}

float falloff(float value) {
    float ret;
    #if defined(Constant)
        ret = 1.0;
    #elif defined(Linear)
        ret = value;
    #elif defined(Spherical)
        ret = sqrt(value * value);
    #elif defined(InverseSpherical)
        value = 1.0 - value;
        ret = 1.0 - sqrt(value * value);
    #elif defined(Cosine)
        ret = (1.0 - cos(value * PI)) * 0.5;
    #endif
    return ret;
}

vec4 blend(vec4 src, vec4 dest) {
    vec3 rgb;
    float a;
    #if defined(Mix)
        a = src.a + dest.a * (1.0 - src.a);
        rgb = (src.rgb * src.a + dest.rgb * dest.a * (1.0 - src.a)) / a;
    #elif defined(Erase)
        rgb = dest.rgb;
        a = dest.a - src.a;
    #elif defined(Add) || defined(Subtract)
        #if defined(Subtract)
            src.a *= -1.0;
        #endif
        //rgb = src.rgb * (1.0 - dest.a) + (src.rgb * src.a + dest.rgb) * dest.a;
        //a = src.a * (1.0 - dest.a) + dest.a;
        rgb = src.rgb * src.a + dest.rgb;
        a = dest.a;
    #elif defined(Multiply) || defined(Divide)
        #if defined(Divide)
            src.a = 1.0 / src.a;
        #endif
        rgb = src.rgb * src.a * dest.rgb;
        a = dest.a;
    #endif
    #if defined(PreserveAlpha)
        return vec4(rgb, dest.a);
    #else
        return vec4(rgb, a);
    #endif
}

void main(void) {
    float weight = (1.0 - gainFunc(gain, biasFunc(bias, falloff(type(texturePos))))) * blendStrength;

    vec4 src = vec4(colour.rgb, colour.a);
    vec4 dest = texture2D(dest, destPos / vec2(64, 64));
    
    src.a *= weight;
    fragment = blend(src, dest);
}
