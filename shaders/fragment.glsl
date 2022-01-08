#include "util.glsl"
#include "buffer.glsl"
#include "compositing.glsl"
#include "blending.glsl"
#include "palette.glsl"

uniform Colour srcTransparent;
//uniform Colour destTransparent;

out layout(location = 0) DEST_VALUE_TYPE fragment;

vec4 blend(const vec4 dest, const vec4 src) {
    return vec4(BLEND_MODE(dest.rgb, src.rgb), src.a);
}

vec4 compose(const vec4 dest, const vec4 src) {
    return unpremultiply(COMPOSE_MODE(dest, src));
}

void main(void) {
    Colour destColour = dest(gl_FragCoord.xy);
    Colour srcColour = src();
    vec4 blended = blend(destColour.rgba, srcColour.rgba);
    vec4 composed = compose(destColour.rgba, blended);
#if defined(DEST_INDEXED)
//    fragment = DEST_VALUE_TYPE(quantiseBruteForce(destPaletteTexture, uint(DEST_PALETTE_FORMAT_SCALE), composed, 0.5, destData.transparent.index));
    fragment = DEST_VALUE_TYPE(quantiseBruteForce(destPaletteTexture, uint(DEST_PALETTE_FORMAT_SCALE), composed, 0.5, INDEX_INVALID));
#else
    fragment = DEST_VALUE_TYPE(fromUnit(composed, DEST_SCALAR_VALUE_TYPE(DEST_FORMAT_SCALE)));
#endif
}
