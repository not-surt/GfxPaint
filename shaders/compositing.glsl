/*vec4 premultiply(const vec4 colour) {
    return vec4(colour.rgb * colour.a, colour.a);
}

vec4 unpremultiply(const vec4 colour) {
    if (colour.a == 0.0) return colour;
    else return vec4(colour.rgb / colour.a, colour.a);
}

vec4 alphaCompositePremultiplied(const vec4 dest, const vec4 src) {
    return src.rgb + dest.rgb * (1.0 - src.a);
}

vec4 alphaComposite(const vec4 dest, const vec4 src) {
    return unpremultiply(alphaCompositePremultiplied(premultiply(dest), premultiply(src)));
}*/

vec4 porterDuff(const vec4 dest, const float destFraction, const vec4 src, const float srcFraction) {
    // NOTE: Output is premultiplied
    return vec4(src.a * srcFraction * src.rgb + dest.a * destFraction * dest.rgb, src.a + dest.a * (1.0 - src.a));
}

vec4 porterDuffClear(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 0.0, src, 0.0);
}

vec4 porterDuffCopy(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 0.0, src, 1.0);
}

vec4 porterDuffDest(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 1.0, src, 0.0);
}

vec4 porterDuffSrcOver(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 1.0 - src.a, src, 1.0);
}

vec4 porterDuffDestOver(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 1.0, src, 1.0 - dest.a);
}

vec4 porterDuffSrcIn(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 0.0, src, dest.a);
}

vec4 porterDuffDestIn(const vec4 dest, const vec4 src) {
    return porterDuff(dest, src.a, src, 0.0);
}

vec4 porterDuffSrcOut(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 0.0, src, 1.0 - dest.a);
}

vec4 porterDuffDestOut(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 1.0 - src.a, src, 0.0);
}

vec4 porterDuffSrcAtop(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 1.0 - src.a, src, dest.a);
}

vec4 porterDuffDestAtop(const vec4 dest, const vec4 src) {
    return porterDuff(dest, src.a, src, 1.0 - dest.a);
}

vec4 porterDuffXor(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 1.0 - src.a, src, 1.0 - dest.a);
}

vec4 porterDuffLighter(const vec4 dest, const vec4 src) {
    return porterDuff(dest, 1.0, src, 1.0);
}
