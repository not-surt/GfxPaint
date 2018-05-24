#include "types.h"

namespace GfxPaint {

QMap<ColourSpace, ColourSpaceInfo> colourSpaceInfo = {
    {ColourSpace::RGB, {"RGB", "rgb", 3, {"Red", "Green", "Blue"}}},
    {ColourSpace::sRGB, {"sRGB", "srgb", 3, {"Red", "Green", "Blue"}}},
    {ColourSpace::XYZ, {"XYZ", "xyz", 3, {"X", "Y", "Z"}}},
    {ColourSpace::xyY, {"xyY", "xyY", 3, {"x", "y", "Y"}}},
    {ColourSpace::HSV, {"HSV", "hsv", 3, {"Hue", "Saturation", "Value"}}},
    {ColourSpace::HSL, {"HSL", "hsl", 3, {"Hue", "Saturation", "Lightness"}}},
    {ColourSpace::HCY, {"HCY", "hcy", 3, {"Hue", "Chroma", "Luminance"}}},
};

} // namespace GfxPaint
