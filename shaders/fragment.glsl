#include <compositing.glsl>
#include <blending.glsl>

uniform Colour srcTransparent;
//uniform Colour destTransparent;

out layout(location = 0) VALUE_TYPE fragment;

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
#if defined(IS_INDEXED) && defined(HAS_PALETTE)
    fragment = VALUE_TYPE(quantiseBruteForce(destPaletteTexture, uint(PALETTE_FORMAT_SCALE), composed, 0.5, destData.transparent.index));
    )";
#elif defined(IS_INDEXED) && !defined(HAS_PALETTE)
    else if (indexed && !paletteFormat.isValid()) src += R"(
    fragment = VALUE_TYPE(fromUnit((composed.r + composed.g + composed.b) / 3.0, SCALAR_VALUE_TYPE(FORMAT_SCALE)));
    )";
#else
    fragment = VALUE_TYPE(fromUnit(composed, SCALAR_VALUE_TYPE(FORMAT_SCALE)));
#endif
}
