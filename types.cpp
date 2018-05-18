#include "types.h"

namespace GfxPaint {

QMap<ColourSpace, ColourSpaceInfo> colourSpaceInfo = {
    {ColourSpace::RGB, {"RGB", "rgb", {"Red", "Green", "Blue"}}},
    {ColourSpace::sRGB, {"sRGB", "srgb", {"Red", "Green", "Blue"}}},
    {ColourSpace::XYZ, {"XYZ", "xyz", {"X", "Y", "Z"}}},
    {ColourSpace::xyY, {"xyY", "xyY", {"x", "y", "Y"}}},
    {ColourSpace::HSV, {"HSV", "hsv", {"Hue", "Saturation", "Value"}}},
    {ColourSpace::HSL, {"HSL", "hsl", {"Hue", "Saturation", "Lightness"}}},
    {ColourSpace::HCY, {"HCY", "hcy", {"Hue", "Chroma", "Luminance"}}},
};

} // namespace GfxPaint
