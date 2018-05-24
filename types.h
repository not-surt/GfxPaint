#ifndef TYPES_H
#define TYPES_H

#include <QImage>

#include "opengl.h"

namespace GfxPaint {

enum class Model {
    SingleVertex,
    ClipQuad,
    UnitQuad,
};

enum class Pattern {
    Checkers,
};


enum class Metric {
    Euclidean,
    Manhattan,
    Chebyshev,
    //Minkoski,
    Minimum,
    Octagonal,
};

enum class Blender {
    Alpha,
    Erase,
    Replace,
    Add,
    Subtract,
    Multiply,
    Divide,
};

enum class ColourSpace {
    RGB,
    sRGB,
    XYZ,
    xyY,
    HSV,
    HSL,
    HCY,
};

typedef std::tuple<ColourSpace, ColourSpace> ColourSpaceConversion;

struct ColourSpaceInfo {
    QString label;
    QString funcName;
    int componentCount;
    QStringList componentLabels;
};

extern QMap<ColourSpace, ColourSpaceInfo> colourSpaceInfo;

} // namespace GfxPaint

#endif // TYPES_H
