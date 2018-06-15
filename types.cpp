#include "type.h"

namespace GfxPaint {

QMap<ColourSpace, ColourSpaceInfo> colourSpaceInfo = {
    {ColourSpace::RGB, {"RGB", "rgb", 3, {"Red", "Green", "Blue"}, {{0.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}}}},
    {ColourSpace::sRGB, {"sRGB", "srgb", 3, {"Red", "Green", "Blue"}, {{0.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}}}},
    {ColourSpace::XYZ, {"XYZ", "xyz", 3, {"X", "Y", "Z"}, {{0.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}}}},
    {ColourSpace::xyY, {"xyY", "xyY", 3, {"x", "y", "Y"}, {{0.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}}}},
    {ColourSpace::HSV, {"HSV", "hsv", 3, {"Hue", "Saturation", "Value"}, {{0.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}}}},
    {ColourSpace::HSL, {"HSL", "hsl", 3, {"Hue", "Saturation", "Lightness"}, {{0.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}}}},
    {ColourSpace::HCY, {"HCY", "hcy", 3, {"Hue", "Chroma", "Luminance"}, {{0.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}}}},
};

} // namespace GfxPaint
