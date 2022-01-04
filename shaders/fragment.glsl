#include "util.glsl"
#include "buffer.glsl"
#include "compositing.glsl"
#include "blending.glsl"
#include "palette.glsl"

uniform Colour srcTransparent;
//uniform Colour destTransparent;

out layout(location = 0) VALUE_TYPE fragment;

vec4 blend(const vec4 dest, const vec4 src) {
    return vec4(BLEND_MODE(dest.rgb, src.rgb), src.a);
}

vec4 compose(const vec4 dest, const vec4 src) {
    return unpremultiply(COMPOSE_MODE(dest, src));
}

//DEST_PALEETE_BUFFER_TYPE(DEST_BUFFER_NAME, DEST_BUFFER_TEXTURE_LOCATION, DEST_BUFFER_SAMPLER_TYPE, DEST_BUFFER_FORMAT_SCALE, DEST_BUFFER_SCALAR_VALUE_TYPE)
//DEST_BUFFER_TYPE(DEST_BUFFER_NAME, DEST_BUFFER_TEXTURE_LOCATION, DEST_BUFFER_SAMPLER_TYPE, DEST_BUFFER_FORMAT_SCALE, DEST_BUFFER_SCALAR_VALUE_TYPE)

void main(void) {
    Colour destColour = dest(gl_FragCoord.xy);
    Colour srcColour = src();
    vec4 blended = blend(destColour.rgba, srcColour.rgba);
    vec4 composed = compose(destColour.rgba, blended);
#if defined(IS_INDEXED) && defined(HAS_PALETTE)
    fragment = VALUE_TYPE(quantiseBruteForce(destPaletteTexture, uint(PALETTE_FORMAT_SCALE), composed, 0.5, destData.transparent.index));
#elif defined(IS_INDEXED) && !defined(HAS_PALETTE)
    fragment = VALUE_TYPE(fromUnit((composed.r + composed.g + composed.b) / 3.0, SCALAR_VALUE_TYPE(FORMAT_SCALE)));
#else
    fragment = VALUE_TYPE(fromUnit(composed, SCALAR_VALUE_TYPE(FORMAT_SCALE)));
#endif
}
