vec4 porterDuff(const vec4 s, const vec4 d, const vec4 b) {
    const float areaSrc = s.a * (1.0 - d.a);
    const float areaDest = d.a * (1.0 - s.a);
    const float areaBoth = s.a * d.a;
    const vec3 colour = areaSrc * s.rgb + areaDest * d.rgb + areaBoth * b.rgb;
    const float as = normalize(s.a);
    const float ad = normalize(d.a);
    const float ab = normalize(b.a);
    const float alpha = areaSrc * as + areaDest * ad + areaBoth * ab;
    return vec4(colour, alpha);
}

vec4 porterDuffClear(const vec4 dest, const vec4 src) {
    return porterDuff(vec4(0.0), vec4(0.0), vec4(0.0));
}

vec4 porterDuffCopy(const vec4 dest, const vec4 src) {
    return porterDuff(src, vec4(0.0), src);
}

vec4 porterDuffDest(const vec4 dest, const vec4 src) {
    return porterDuff(vec4(0.0), dest, dest);
}

vec4 porterDuffSrcOver(const vec4 dest, const vec4 src) {
    return porterDuff(src, dest, src);
}

vec4 porterDuffDestOver(const vec4 dest, const vec4 src) {
    return porterDuff(src, dest, dest);
}

vec4 porterDuffSrcIn(const vec4 dest, const vec4 src) {
    return porterDuff(vec4(0.0), vec4(0.0), src);
}

vec4 porterDuffDestIn(const vec4 dest, const vec4 src) {
    return porterDuff(vec4(0.0), vec4(0.0), dest);
}

vec4 porterDuffSrcOut(const vec4 dest, const vec4 src) {
    return porterDuff(src, vec4(0.0), vec4(0.0));
}

vec4 porterDuffDestOut(const vec4 dest, const vec4 src) {
    return porterDuff(vec4(0.0), dest, vec4(0.0));
}

vec4 porterDuffSrcAtop(const vec4 dest, const vec4 src) {
    return porterDuff(vec4(0.0), dest, src);
}

vec4 porterDuffDestAtop(const vec4 dest, const vec4 src) {
    return porterDuff(src, vec4(0.0), dest);
}

vec4 porterDuffXor(const vec4 dest, const vec4 src) {
    return porterDuff(src, dest, vec4(0.0));
}
