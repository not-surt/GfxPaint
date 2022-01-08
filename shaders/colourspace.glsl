#if !defined(COLOURSPACE_GLSL)
#define COLOURSPACE_GLSL

#include "ColorSpaces.inc.glsl"

// Passthrough converters
vec3 rgb_to_rgb(vec3 rgb) { return rgb; }
vec3 srgb_to_srgb(vec3 srgb) { return srgb; }
vec3 xyz_to_xyz(vec3 xyz) { return xyz; }
vec3 xyY_to_xyY(vec3 xyY) { return xyY; }
vec3 hcv_to_hcv(vec3 hcv) { return hcv; }
vec3 hsv_to_hsv(vec3 hsv) { return hsv; }
vec3 hsl_to_hsl(vec3 hsl) { return hsl; }
vec3 hcy_to_hcy(vec3 hcy) { return hcy; }
vec3 ycbcr_to_ycbcr(vec3 ycbcr) { return ycbcr; }

#endif // COLOURSPACE_GLSL
