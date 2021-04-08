#include "rendermanager.h"

#include <QFileInfo>
#include <cstring>
#include "utils.h"
#include "application.h"

namespace GfxPaint {

const QMatrix4x4 RenderManager::unitToClipTransform = ([](){
        static const GLfloat data[] = {
            2.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 1.0f,
        };
        QMatrix4x4 matrix;
        std::memcpy(matrix.data(), data, sizeof(data));
        return matrix;
    })();

const QMatrix4x4 RenderManager::clipToUnitTransform(RenderManager::unitToClipTransform.inverted());

const QMatrix4x4 RenderManager::flipTransform = ([](){
    static const GLfloat data[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    QMatrix4x4 matrix;
    std::memcpy(matrix.data(), data, sizeof(data));
    return matrix;
    })();

const QMap<ColourSpaceConversion, QString> RenderManager::colourSpaceConversionShaderFunctionNames = {
    {{ColourSpace::RGB, ColourSpace::sRGB}, "rgb_to_srgb"},
    {{ColourSpace::RGB, ColourSpace::XYZ}, "rgb_to_xyz"},
    {{ColourSpace::RGB, ColourSpace::xyY}, "rgb_to_xyY"},
    {{ColourSpace::RGB, ColourSpace::HSV}, "rgb_to_hsv"},
    {{ColourSpace::RGB, ColourSpace::HSL}, "rgb_to_hsl"},
    {{ColourSpace::RGB, ColourSpace::HCY}, "rgb_to_hcy"},

    {{ColourSpace::sRGB, ColourSpace::RGB}, "srgb_to_rgb"},
    {{ColourSpace::sRGB, ColourSpace::XYZ}, "srgb_to_xyz"},
    {{ColourSpace::sRGB, ColourSpace::xyY}, "srgb_to_xyY"},
    {{ColourSpace::sRGB, ColourSpace::HSV}, "srgb_to_hsv"},
    {{ColourSpace::sRGB, ColourSpace::HSL}, "srgb_to_hsl"},
    {{ColourSpace::sRGB, ColourSpace::HCY}, "srgb_to_hcy"},

    {{ColourSpace::XYZ, ColourSpace::RGB}, "xyz_to_rgb"},
    {{ColourSpace::XYZ, ColourSpace::sRGB}, "xyz_to_srgb"},
    {{ColourSpace::XYZ, ColourSpace::xyY}, "xyz_to_xyY"},
    {{ColourSpace::XYZ, ColourSpace::HSV}, "xyz_to_hsv"},
    {{ColourSpace::XYZ, ColourSpace::HSL}, "xyz_to_hsl"},
    {{ColourSpace::XYZ, ColourSpace::HCY}, "xyz_to_hcy"},

    {{ColourSpace::xyY, ColourSpace::RGB}, "xyY_to_rgb"},
    {{ColourSpace::xyY, ColourSpace::sRGB}, "xyY_to_srgb"},
    {{ColourSpace::xyY, ColourSpace::XYZ}, "xyY_to_xyz"},
    {{ColourSpace::xyY, ColourSpace::HSV}, "xyY_to_hsv"},
    {{ColourSpace::xyY, ColourSpace::HSL}, "xyY_to_hsl"},
    {{ColourSpace::xyY, ColourSpace::HCY}, "xyY_to_hcy"},

    {{ColourSpace::HSV, ColourSpace::RGB}, "hsv_to_rgb"},
    {{ColourSpace::HSV, ColourSpace::sRGB}, "hsv_to_srgb"},
    {{ColourSpace::HSV, ColourSpace::XYZ}, "hsv_to_xyz"},
    {{ColourSpace::HSV, ColourSpace::xyY}, "hsv_to_xyY"},
    {{ColourSpace::HSV, ColourSpace::HSL}, "hsv_to_hsl"},
    {{ColourSpace::HSV, ColourSpace::HCY}, "hsv_to_hcy"},

    {{ColourSpace::HSL, ColourSpace::RGB}, "hsl_to_rgb"},
    {{ColourSpace::HSL, ColourSpace::sRGB}, "hsl_to_srgb"},
    {{ColourSpace::HSL, ColourSpace::XYZ}, "hsl_to_xyz"},
    {{ColourSpace::HSL, ColourSpace::xyY}, "hsl_to_xyY"},
    {{ColourSpace::HSL, ColourSpace::HSV}, "hsl_to_hsv"},
    {{ColourSpace::HSL, ColourSpace::HCY}, "hsl_to_hcy"},

    {{ColourSpace::HCY, ColourSpace::RGB}, "hcy_to_rgb"},
    {{ColourSpace::HCY, ColourSpace::sRGB}, "hcy_to_srgb"},
    {{ColourSpace::HCY, ColourSpace::XYZ}, "hcy_to_xyz"},
    {{ColourSpace::HCY, ColourSpace::xyY}, "hcy_to_xyY"},
    {{ColourSpace::HCY, ColourSpace::HSV}, "hcy_to_hsv"},
    {{ColourSpace::HCY, ColourSpace::HSL}, "hcy_to_hsl"},
};

const QList<RenderManager::DistanceMetricInfo> RenderManager::distanceMetrics = {
    {"Euclidean", "distanceEuclidean"},
    {"Manhattan", "distanceManhattan"},
    {"Chebyshev", "distanceChebyshev"},
    {"Minimum", "distanceMinimum"},
    {"Octagonal", "distanceOctagonal"},
};

const QList<RenderManager::BlendModeInfo> RenderManager::blendModes = {
    {"Normal", "blendNormal"},
    {"Multiply", "blendMultiply"},
    {"Screen", "blendScreen"},
    {"Overlay", "blendOverlay"},
    {"Darken", "blendDarken"},
    {"Lighten", "blendLighten"},
    {"Colour Dodge", "blendColourDodge"},
    {"Colour Burn", "blendColourBurn"},
    {"Hard Light", "blendHardLight"},
    {"Soft Light", "blendSoftLight"},
    {"Difference", "blendDifference"},
    {"Exclusion", "blendExclusion"},
    {"Hue", "blendHue"},
    {"Saturation", "blendSaturation"},
    {"Colour", "blendColour"},
    {"Luminosity", "blendLuminosity"},
};

const QList<RenderManager::ComposeModeInfo> RenderManager::composeModes = {
    {"Clear", "porterDuffClear"},
    {"Copy", "porterDuffCopy"},
    {"Destination", "porterDuffDest"},
    {"Source Over", "porterDuffSrcOver"},
    {"Destination Over", "porterDuffDestOver"},
    {"Source In", "porterDuffSrcIn"},
    {"Destination In", "porterDuffDestIn"},
    {"Source Out", "porterDuffSrcOut"},
    {"Destination Out", "porterDuffDestOut"},
    {"Source Atop", "porterDuffSrcAtop"},
    {"Destination Atop", "porterDuffDestAtop"},
    {"Xor", "porterDuffXor"},
};
const int RenderManager::composeModeDefault = 3;

RenderManager::RenderManager() :
    OpenGL(),
    surface(), context(),
    logger(),
    vao(),
    models(), programManager()
{
    // Create offscreen render context
    // OpenGL ES
    QSurfaceFormat formatGLES;
    formatGLES.setRenderableType(QSurfaceFormat::OpenGLES);
    formatGLES.setVersion(std::get<0>(openGLESVersion), std::get<1>(openGLESVersion));
#ifdef QT_DEBUG
    formatGLES.setOption(QSurfaceFormat::DebugContext);
#endif
    // Desktop OpenGL
    QSurfaceFormat formatGL;
    formatGL.setRenderableType(QSurfaceFormat::OpenGL);
    formatGL.setVersion(std::get<0>(openGLVersion), std::get<1>(openGLVersion));
    formatGL.setProfile(QSurfaceFormat::CoreProfile);
#ifdef QT_DEBUG
    formatGL.setOption(QSurfaceFormat::DebugContext);
#endif
    // Try to create OpenGL ES context
    surface.setFormat(formatGLES);
    Q_ASSERT(QThread::currentThread() == QGuiApplication::instance()->thread());
    surface.create();
    context.setFormat(surface.format());
    context.setShareContext(QOpenGLContext::globalShareContext());
    const bool createContextSuccess = context.create();
//    const bool createContextSuccess = false;
    // If OpenGL ES context creation not successfull try to fall back to comparable desktop OpenGL context
    if (!createContextSuccess) {
        surface.destroy();
        surface.setFormat(formatGL);
        surface.create();
        context.setFormat(surface.format());
        const bool createContextSuccess = context.create();
        Q_ASSERT(createContextSuccess);
    }
    qDebug() << "Is OpenGL ES?" << context.isOpenGLES();//////////////////////////////
    qDebug() << context.format();//////////////////////////////

//    surface.create();
//    context.setShareContext(QOpenGLContext::globalShareContext());
//    context.create();

//    if (!context.isValid() || context.format().version() < QSurfaceFormat::defaultFormat().version()) {
//        qDebug() << "Invalid context";
//        exit(EXIT_FAILURE);
//    }

    ContextBinder contextBinder(&context, &surface);

    logger.initialize();
    QObject::connect(&logger, &QOpenGLDebugLogger::messageLogged, [](const QOpenGLDebugMessage & debugMessage) {
        qDebug() << "OpenGL:" << debugMessage.message();
    });
    logger.disableMessages(QOpenGLDebugMessage::AnySource, QOpenGLDebugMessage::AnyType, QOpenGLDebugMessage::NotificationSeverity);
    logger.disableMessages(QOpenGLDebugMessage::AnySource, QOpenGLDebugMessage::PerformanceType, QOpenGLDebugMessage::AnySeverity);
    logger.startLogging();

    OpenGL::initializeOpenGLFunctions();

    vao.create();
    vao.bind();

    models["sliderMarker"] = new Model(GL_TRIANGLES, {2, 4,}, {
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        }, {
            0, 1, 2,
            3, 4, 5,
        }, {3, 3,});

    models["planeMarker"] = new Model(GL_TRIANGLES, {2, 4,}, {
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        }, {
            0, 1, 2,
            3, 4, 5,
        }, {3, 3,});
}

RenderManager::~RenderManager()
{
    {
        ContextBinder contextBinder(&context, &surface);

        qDeleteAll(models);

        logger.stopLogging();

        vao.destroy();
    }
}

QSurfaceFormat RenderManager::defaultFormat()
{
    QSurfaceFormat format;
    format.setVersion(std::get<0>(openGLVersion), std::get<1>(openGLVersion));
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    return format;
}

QString RenderManager::glslVersionString() const
{
    QString src;
    src+= "#version " + QString::number(surface.format().majorVersion()) + QString::number(surface.format().minorVersion()) + "0" + (surface.format().renderableType() == QSurfaceFormat::OpenGLES ? " es" : " core") + "\n";
    if (surface.format().renderableType() == QSurfaceFormat::OpenGLES) {
        src +=
R"(
precision highp float;
precision highp int;
precision highp sampler2D;
precision highp isampler2D;
precision highp usampler2D;
//precision highp image2D;
//precision highp iimage2D;
//precision highp uimage2D;
//precision highp imageBuffer;
//precision highp iimageBuffer;
//precision highp uimageBuffer;
)";
    }
    return src;
}

QString RenderManager::headerShaderPart()
{
    QString src;
    src += qApp->renderManager.glslVersionString();
    src += fileToString(":/shaders/header.glsl");
    src += fileToString(":/shaders/util.glsl");
    for (auto key : colourSpaceInfo.keys()) {
        QString str =
R"(
vec3 $COLOURSPACE_to_$COLOURSPACE(vec3 rgb) {
    return rgb;
}
)";
        stringMultiReplace(str, {
            {"$COLOURSPACE", colourSpaceInfo[key].funcName},
        });
        src += str;
    }
    return src;
}

QString RenderManager::attributelessShaderPart(const AttributelessModel model)
{
    const QMap<AttributelessModel, QString> models = {
        { AttributelessModel::SingleVertex,
R"(
const vec2 vertices[1] = vec2[](
    vec2(0.0, 0.0)
);
)"
        },
        { AttributelessModel::ClipQuad,
R"(
const vec2 vertices[4] = vec2[](
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);
)"
        },
        { AttributelessModel::UnitQuad,
R"(
const vec2 vertices[4] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);
)"
        },
    };
    QString src;
    src += models[model];
    return src;
}

QString RenderManager::modelVertexMainShaderPart()
{
    QString src;
    src +=
R"(
uniform mat4 matrix;

in layout(location = 0) vec2 vertexPos;
in layout(location = 1) vec4 vertexColour;

out layout(location = 0) vec2 pos;
out layout(location = 1) vec4 colour;

void main(void) {
    pos = vec2(0.0, 0.0);
    colour = vertexColour;
    gl_Position = matrix * vec4(vertexPos, 0.0, 1.0);
}
)";
    return src;
}

QString RenderManager::vertexMainShaderPart()
{
    QString src;
    src +=
R"(
uniform mat4 matrix;

uniform ivec2 srcRectPos;
uniform ivec2 srcRectSize;

out layout(location = 0) vec2 pos;

void main(void) {
    vec2 vertexPos = vertices[gl_VertexID];
    pos = vec2(srcRectPos) + vertexPos * vec2(srcRectSize);
    gl_Position = matrix * vec4(vertexPos, 0.0, 1.0);
}
)";
    return src;
}

QString RenderManager::patternShaderPart(const QString &name, const Pattern pattern)
{
    QString src;
    src +=
R"(
const float base = 7.0 / 16.0;
const float offset = 1.0 / 16.0;
const float light = base + offset;
const float dark = base - offset;

vec4 lightColour = vec4(vec3(light), 1.0);
vec4 darkColour = vec4(vec3(dark), 1.0);

vec4 $NAME(const vec2 pos) {
    bool alternate = !((mod(pos.x, 2.0) >= 1.0) == (mod(pos.y, 2.0) >= 1.0));
    return alternate ? lightColour : darkColour;
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
    });
    return src;
}

QString RenderManager::paletteShaderPart(const QString &name, const GLint paletteTextureLocation, const Buffer::Format paletteFormat)
{
    QString src;
    src +=
R"(
uniform layout(location = $LOCATION) $SAMPLER_TYPE $NAMEPaletteTexture;

vec4 $NAMEPalette(const uint index) {
    return toUnit(paletteSample($NAMEPaletteTexture, index), $SCALAR_VALUE_TYPE($FORMAT_SCALE));
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
        {"$LOCATION", QString::number(paletteTextureLocation)},
        {"$SAMPLER_TYPE", paletteFormat.shaderSamplerType()},
        {"$SCALAR_VALUE_TYPE", paletteFormat.shaderScalarValueType()},
        {"$FORMAT_SCALE", QString::number(paletteFormat.scale())},
    });
    return src;
}

QString RenderManager::bufferShaderPart(const QString &name, const GLint uniformBlockBinding, const GLint bufferTextureLocation, const Buffer::Format bufferFormat, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat)
{
    Q_ASSERT(!indexed || paletteFormat.isValid());

    QString src;
    if (indexed && paletteFormat.isValid()) src += paletteShaderPart(name, paletteTextureLocation, paletteFormat);
    src +=
R"(
uniform layout(location = $TEXTURE_LOCATION) $SAMPLER_TYPE $NAMETexture;

layout(std140, binding = $UNIFORM_BLOCK_BINDING) uniform $NAMEUniformData {
    mat4 matrix;
    Colour transparent;
} $NAMEData;

Colour $NAME(const vec2 pos) {
    Colour colour = COLOUR_INVALID;
//    Colour transparent = $NAMEData.transparent;
)";
    if (indexed && paletteFormat.isValid()) src +=
R"(
    colour.index = texelFetch($NAMETexture, ivec2(floor(pos)), 0).x;
//    colour.rgba = (colour.index == transparent.index ? vec4(0.0) : $NAMEPalette(colour.index));
    colour.rgba = $NAMEPalette(colour.index);
)";
    else if (indexed && !paletteFormat.isValid()) src +=
R"(
    float grey = toUnit(texelFetch($NAMETexture, ivec2(floor(pos)), 0).x, $SCALAR_VALUE_TYPE($FORMAT_SCALE));
    colour.rgba = (colour.index == transparent.index ? vec4(0.0) : vec4(vec3(grey), 1.0));
)";
    else src +=
R"(
    vec4 texelRgba = toUnit(texelFetch($NAMETexture, ivec2(floor(pos)), 0), $SCALAR_VALUE_TYPE($FORMAT_SCALE));
//    colour.rgba = (texelRgba == transparent.rgba ? vec4(0.0) : texelRgba);
//    if (transparent.rgba != RGBA_INVALID) {
//        colour.rgba = (texelRgba == transparent.rgba ? vec4(0.0) : texelRgba);
//    }
//    else {
        colour.rgba =  texelRgba;
//    }
)";
    src +=
R"(
    return colour;
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
        {"$TEXTURE_LOCATION", QString::number(bufferTextureLocation)},
        {"$SAMPLER_TYPE", bufferFormat.shaderSamplerType()},
        {"$FORMAT_SCALE", QString::number(bufferFormat.scale())},
        {"$SCALAR_VALUE_TYPE", bufferFormat.shaderScalarValueType()},
        {"$UNIFORM_BLOCK_BINDING", QString::number(uniformBlockBinding)},
    });
    return src;
}

QString RenderManager::modelFragmentShaderPart(const QString &name)
{
    QString src;
    src +=
R"(
//in layout(location = 0) vec2 pos;
in layout(location = 1) vec4 colour;

Colour $NAME(const vec2 pos) {
    return Colour(colour, INDEX_INVALID);
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
    });
    return src;
}

QString RenderManager::dabShaderPart(const QString &name, const Brush::Dab::Type type, const int metric)
{
    const QMap<Brush::Dab::Type, QString> types = {
        { Brush::Dab::Type::Pixel,
R"(
float $NAMEBrush(const vec2 pos) {
    return 0.0;
}
)"
        },
        { Brush::Dab::Type::Distance,
R"(
float $NAMEBrush(const vec2 pos) {
    return $METRIC(pos);
}
)"
        },
        { Brush::Dab::Type::Buffer,
R"(
float $NAMEBrush(const vec2 pos) {
    return 0.0;
}
)"
        },
    };
    QString src;
    if (type == Brush::Dab::Type::Distance) {
        src += fileToString(":/shaders/distance.glsl");
    }
    src += types[type];
    src +=
R"(
layout(std140, binding = 0) uniform Data {
    mat4 matrix;
    Colour colour;
    float hardness;
    float opacity;
} data;
uniform float $NAMEHardness;
uniform float $NAMEOpacity;
uniform vec4 $NAMEColour;
Colour $NAME(const vec2 pos) {
    float weight;
    weight = clamp(1.0 - $NAMEBrush(pos), 0.0, 1.0);
    weight = clamp(weight * (1.0 / (1.0 - $NAMEHardness)), 0.0, 1.0);
    weight *= $NAMEOpacity;
    return Colour(vec4($NAMEColour.rgb, $NAMEColour.a * weight), INDEX_INVALID);
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
        {"$METRIC", distanceMetrics[metric].functionName},
    });
    return src;
}

QString RenderManager::colourPlaneShaderPart(const QString &name, const ColourSpace colourSpace, const bool useXAxis, const bool useYAxis, const bool quantise, const GLint quantisePaletteTextureLocation, const Buffer::Format quantisePaletteFormat)
{
    QString src;
    src += fileToString(":/shaders/ColorSpaces.inc.glsl");
    if (quantise) {
        src += fileToString(":/shaders/palette.glsl");
        src += paletteShaderPart("quantise", quantisePaletteTextureLocation, quantisePaletteFormat);
    }
    src +=
R"(
layout(std140, binding = 0) uniform Data {
    Colour colour;
    ivec2 components;
} data;

vec4 colourPlane(const vec4 colour, const ivec2 components, const vec2 pos) {
    vec4 colour00 = colour;
    vec4 colour01 = colour;
    vec4 colour10 = colour;
    vec4 colour11 = colour;
)";
    if (useXAxis) src +=
R"(
    colour00[components[0]] = 0.0;
    colour01[components[0]] = 0.0;
    colour10[components[0]] = 1.0;
    colour11[components[0]] = 1.0;
)";
    if (useYAxis) src +=
R"(
    colour00[components[1]] = 0.0;
    colour01[components[1]] = 1.0;
    colour10[components[1]] = 0.0;
    colour11[components[1]] = 1.0;
)";
    src +=
R"(
    return bilinear(colour00, colour01, colour10, colour11, pos);
}

Colour $NAME(const vec2 pos) {
    uint index = INDEX_INVALID;
    vec4 col = data.colour.rgba;
    col = vec4(rgb_to_$COLOURSPACE(col.rgb), col.a);
    col = colourPlane(col, data.components, pos.xy);
    col = vec4($COLOURSPACE_to_rgb(col.rgb), col.a);
)";
    if (quantise && quantisePaletteFormat.isValid()) src +=
R"(
    index = quantiseBruteForce(quantisePaletteTexture, uint($PALETTE_FORMAT_SCALE), vec4(col.rgb, 1.0), 0.5, INDEX_INVALID);
    col = vec4(quantisePalette(index).rgb, col.a);
)";
    src +=
R"(
    return Colour(col, index);
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
        {"$COLOURSPACE", colourSpaceInfo[colourSpace].funcName},
        {"$PALETTE_FORMAT_SCALE", QString::number(quantise ? quantisePaletteFormat.scale() : 0)},
    });
    return src;
}

QString RenderManager::colourPaletteShaderPart(const QString &name)
{
    QString src;
    src +=
R"(
uniform ivec2 cells;

Colour $NAME(const vec2 pos) {
    ivec2 cell = ivec2(floor(pos / vec2(cells)));
    int index = cell.x + cell.y * cells.x;
    if (cell.x >= 0 && cell.x < cells.x &&
        cell.y >= 0 && cell.y < cells.y &&
        index >= 0 && index < int(paletteSize($NAMEPalettePaletteTexture)))
        return Colour(vec4($NAMEPalettePalette(Index(index))), Index(index));
    else return Colour(vec4(0.0, 0.0, 0.0, 0.0), INDEX_INVALID);
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
    });
    return src;
}

QString RenderManager::fragmentMainShaderPart(const Buffer::Format format, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat, const int blendMode, const int composeMode)
{
    QString src;
    src += fileToString(":/shaders/compositing.glsl");
    src += fileToString(":/shaders/blending.glsl");
    if (indexed && paletteFormat.isValid()) src += fileToString(":/shaders/palette.glsl");
    src +=
R"(
in layout(location = 0) vec2 pos;

uniform Colour srcTransparent;
//uniform Colour destTransparent;

out layout(location = 0) $VALUE_TYPE fragment;

vec4 blend(const vec4 dest, const vec4 src) {
    return vec4($BLEND_MODE(dest.rgb, src.rgb), src.a);
}

vec4 compose(const vec4 dest, const vec4 src) {
    return unpremultiply($COMPOSE_MODE(dest, src));
}

void main(void) {
    Colour destColour = dest(gl_FragCoord.xy);
    Colour srcColour = src(pos);
    vec4 blended = blend(destColour.rgba, srcColour.rgba);
    vec4 composed = compose(destColour.rgba, blended);
)";
    if (indexed && paletteFormat.isValid()) src +=
R"(
    fragment = $VALUE_TYPE(quantiseBruteForce(destPaletteTexture, uint($PALETTE_FORMAT_SCALE), composed, 0.5, destData.transparent.index));
)";
    else if (indexed && !paletteFormat.isValid()) src +=
R"(
    fragment = $VALUE_TYPE(fromUnit((composed.r + composed.g + composed.b) / 3.0, $SCALAR_VALUE_TYPE($FORMAT_SCALE)));
)";
    else src +=
R"(
    fragment = $VALUE_TYPE(fromUnit(composed, $SCALAR_VALUE_TYPE($FORMAT_SCALE)));
)";
    src +=
R"(
}
)";
    stringMultiReplace(src, {
        {"$VALUE_TYPE", format.shaderValueType()},
        {"$FORMAT_SCALE", QString::number(format.scale())},
        {"$PALETTE_FORMAT_SCALE", QString::number(indexed && paletteFormat.isValid() ? paletteFormat.scale() : 1.0)},
        {"$BLEND_MODE", RenderManager::blendModes[blendMode].functionName},
        {"$COMPOSE_MODE", RenderManager::composeModes[composeMode].functionName},
        {"$SCALAR_VALUE_TYPE", format.shaderScalarValueType()},
    });
    return src;
}

QString RenderManager::widgetOutputShaderPart()
{
    QString src;
    src +=
R"(
in layout(location = 0) vec2 pos;

out layout(location = 0) vec4 fragment;

void main(void) {
    fragment = premultiply(src(pos).rgba);
}
)";
    return src;
}

void RenderManager::bindBufferShaderPart(QOpenGLShaderProgram &program, const QString &name, const GLint bufferTextureLocation, const Buffer *const buffer)
{
    glUniform1i(program.uniformLocation(name + "Texture"), bufferTextureLocation);
    buffer->bindTextureUnit(bufferTextureLocation);
}

void RenderManager::bindIndexedBufferShaderPart(QOpenGLShaderProgram &program, const QString &name, const GLint bufferTextureLocation, const Buffer *const buffer, const bool indexed, const GLint paletteTextureLocation, const Buffer *const palette)
{
    bindBufferShaderPart(program, name, bufferTextureLocation, buffer);
    if (indexed && palette) {
        bindBufferShaderPart(program, name + "Palette", paletteTextureLocation, palette);
    }
}

} // namespace GfxPaint
