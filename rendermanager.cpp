#include "rendermanager.h"

#include <QFileInfo>
#include "utils.h"
#include "application.h"

namespace GfxPaint {

const QTransform RenderManager::unitToClipTransform(
        2.0, 0.0, 0.0,
        0.0, 2.0, 0.0,
        -1.0, -1.0, 1.0);

const QTransform RenderManager::clipToUnitTransform(RenderManager::unitToClipTransform.inverted());

const QTransform RenderManager::flipTransform(
        1.0, 0.0, 0.0,
        0.0, -1.0, 0.0,
        0.0, 0.0, 1.0);

const QMap<ColourSpaceConversion, QString> RenderManager::colourSpaceConversionShaderFunctionNames = {
    {ColourSpaceConversion(ColourSpace::RGB, ColourSpace::sRGB), "rgb_to_srgb"},
    {ColourSpaceConversion(ColourSpace::RGB, ColourSpace::XYZ), "rgb_to_xyz"},
    {ColourSpaceConversion(ColourSpace::RGB, ColourSpace::xyY), "rgb_to_xyY"},
    {ColourSpaceConversion(ColourSpace::RGB, ColourSpace::HSV), "rgb_to_hsv"},
    {ColourSpaceConversion(ColourSpace::RGB, ColourSpace::HSL), "rgb_to_hsl"},
    {ColourSpaceConversion(ColourSpace::RGB, ColourSpace::HCY), "rgb_to_hcy"},

    {ColourSpaceConversion(ColourSpace::sRGB, ColourSpace::RGB), "srgb_to_rgb"},
    {ColourSpaceConversion(ColourSpace::sRGB, ColourSpace::XYZ), "srgb_to_xyz"},
    {ColourSpaceConversion(ColourSpace::sRGB, ColourSpace::xyY), "srgb_to_xyY"},
    {ColourSpaceConversion(ColourSpace::sRGB, ColourSpace::HSV), "srgb_to_hsv"},
    {ColourSpaceConversion(ColourSpace::sRGB, ColourSpace::HSL), "srgb_to_hsl"},
    {ColourSpaceConversion(ColourSpace::sRGB, ColourSpace::HCY), "srgb_to_hcy"},

    {ColourSpaceConversion(ColourSpace::XYZ, ColourSpace::RGB), "xyz_to_rgb"},
    {ColourSpaceConversion(ColourSpace::XYZ, ColourSpace::sRGB), "xyz_to_srgb"},
    {ColourSpaceConversion(ColourSpace::XYZ, ColourSpace::xyY), "xyz_to_xyY"},
    {ColourSpaceConversion(ColourSpace::XYZ, ColourSpace::HSV), "xyz_to_hsv"},
    {ColourSpaceConversion(ColourSpace::XYZ, ColourSpace::HSL), "xyz_to_hsl"},
    {ColourSpaceConversion(ColourSpace::XYZ, ColourSpace::HCY), "xyz_to_hcy"},

    {ColourSpaceConversion(ColourSpace::xyY, ColourSpace::RGB), "xyY_to_rgb"},
    {ColourSpaceConversion(ColourSpace::xyY, ColourSpace::sRGB), "xyY_to_srgb"},
    {ColourSpaceConversion(ColourSpace::xyY, ColourSpace::XYZ), "xyY_to_xyz"},
    {ColourSpaceConversion(ColourSpace::xyY, ColourSpace::HSV), "xyY_to_hsv"},
    {ColourSpaceConversion(ColourSpace::xyY, ColourSpace::HSL), "xyY_to_hsl"},
    {ColourSpaceConversion(ColourSpace::xyY, ColourSpace::HCY), "xyY_to_hcy"},

    {ColourSpaceConversion(ColourSpace::HSV, ColourSpace::RGB), "hsv_to_rgb"},
    {ColourSpaceConversion(ColourSpace::HSV, ColourSpace::sRGB), "hsv_to_srgb"},
    {ColourSpaceConversion(ColourSpace::HSV, ColourSpace::XYZ), "hsv_to_xyz"},
    {ColourSpaceConversion(ColourSpace::HSV, ColourSpace::xyY), "hsv_to_xyY"},
    {ColourSpaceConversion(ColourSpace::HSV, ColourSpace::HSL), "hsv_to_hsl"},
    {ColourSpaceConversion(ColourSpace::HSV, ColourSpace::HCY), "hsv_to_hcy"},

    {ColourSpaceConversion(ColourSpace::HSL, ColourSpace::RGB), "hsl_to_rgb"},
    {ColourSpaceConversion(ColourSpace::HSL, ColourSpace::sRGB), "hsl_to_srgb"},
    {ColourSpaceConversion(ColourSpace::HSL, ColourSpace::XYZ), "hsl_to_xyz"},
    {ColourSpaceConversion(ColourSpace::HSL, ColourSpace::xyY), "hsl_to_xyY"},
    {ColourSpaceConversion(ColourSpace::HSL, ColourSpace::HSV), "hsl_to_hsv"},
    {ColourSpaceConversion(ColourSpace::HSL, ColourSpace::HCY), "hsl_to_hcy"},

    {ColourSpaceConversion(ColourSpace::HCY, ColourSpace::RGB), "hcy_to_rgb"},
    {ColourSpaceConversion(ColourSpace::HCY, ColourSpace::sRGB), "hcy_to_srgb"},
    {ColourSpaceConversion(ColourSpace::HCY, ColourSpace::XYZ), "hcy_to_xyz"},
    {ColourSpaceConversion(ColourSpace::HCY, ColourSpace::xyY), "hcy_to_xyY"},
    {ColourSpaceConversion(ColourSpace::HCY, ColourSpace::HSV), "hcy_to_hsv"},
    {ColourSpaceConversion(ColourSpace::HCY, ColourSpace::HSL), "hcy_to_hsl"},
};

const QList<RenderManager::DistanceMetricInfo> RenderManager::distanceMetrics = {
    {"Euclidean", "distanceEuclidean"},
    {"Manhattan", "distanceManhattan"},
    {"Chebyshev", "distanceChebyshev"},
    {"Minimum", "distanceMinimum"},
    {"Octagonal", "distanceOctagonal"},
};

const QList<RenderManager::intInfo> RenderManager::blendModes = {
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
    programManager()
{
    surface.create();
    context.setShareContext(QOpenGLContext::globalShareContext());
    context.create();

    if (!context.isValid() || context.format().version() < QSurfaceFormat::defaultFormat().version()) {
        qDebug() << "Invalid context";
        exit(EXIT_FAILURE);
    }

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
}

RenderManager::~RenderManager()
{
    {
        ContextBinder contextBinder(&context, &surface);

        logger.stopLogging();

        vao.destroy();
    }
}

QString RenderManager::headerShaderPart()
{
    QString src;
    src +=
R"(
#version %1 core
)";
    src += fileToString(":/shaders/util.glsl");
    return src.arg(OPENGL_GLSL_VERSION_STRING);
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

QString RenderManager::geometryVertexShaderPart()
{
    QString src;
    src +=
R"(
uniform mat3 matrix;

in layout(location = 0) vec2 vertexPos;
in layout(location = 1) vec4 vertexColour;

out layout(location = 0) vec2 pos;
out layout(location = 1) vec4 colour;

void main(void) {
    pos = vec2(0.0, 0.0);
    colour = vertexColour;
    gl_Position = vec4(matrix * vec3(vertexPos, 1.0), 1.0);
}
)";
    return src;
}

QString RenderManager::vertexMainShaderPart()
{
    QString src;
    src +=
R"(
uniform mat3 matrix;

uniform ivec2 srcRectPos = ivec2(0.0, 0.0);
uniform ivec2 srcRectSize = ivec2(1.0, 1.0);

out layout(location = 0) vec2 pos;

void main(void) {
    const vec2 vertexPos = vertices[gl_VertexID];
    pos = vec2(srcRectPos) + vertexPos * vec2(srcRectSize);
    gl_Position = vec4(matrix * vec3(vertexPos, 1.0), 1.0);
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

uniform vec4 lightColour = vec4(vec3(light), 1.0);
uniform vec4 darkColour = vec4(vec3(dark), 1.0);

vec4 %1(const vec2 pos) {
    const bool alternate = !((mod(pos.x, 2.0) >= 1.0) == (mod(pos.y, 2.0) >= 1.0));
    return alternate ? lightColour : darkColour;
}
)";
    stringMultiReplace(src, {
        {"%1", name},
    });
    return src;
}

QString RenderManager::paletteShaderPart(const QString &name, const GLint paletteTextureLocation, const Buffer::Format paletteFormat)
{
    QString src;
    src +=
R"(
uniform layout(location = %2) %3 %1PaletteTexture;

vec4 %1Palette(const uint index) {
    return vec4(texelFetch(%1PaletteTexture, ivec2(index, 0))) / float(%4);
}
)";
    stringMultiReplace(src, {
        {"%1", name},
        {"%2", QString::number(paletteTextureLocation)},
        {"%3", paletteFormat.shaderSamplerType()},
        {"%4", QString::number(paletteFormat.scale())},
    });
    return src;
}

QString RenderManager::bufferShaderPart(const QString &name, const GLint bufferTextureLocation, const Buffer::Format bufferFormat, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat)
{
    QString src;
    if (indexed && paletteFormat.isValid()) src += paletteShaderPart(name, paletteTextureLocation, paletteFormat);
    src +=
R"(
uniform layout(location = %2) %3 %1Texture;

vec4 %1(const vec2 pos) {
)";
    if (indexed && paletteFormat.isValid()) src +=
R"(
    const uint index = texelFetch(%1Texture, ivec2(floor(pos))).x;
    const vec4 colour = %1Palette(index);
)";
    else if (indexed && !paletteFormat.isValid()) src +=
R"(
    const float grey = texelFetch(%1Texture, ivec2(floor(pos))).x / float(%4);
    const vec4 colour = vec4(grey, grey, grey, 1.0);
)";
    else src +=
R"(
    const vec4 colour = vec4(texelFetch(%1Texture, ivec2(floor(pos)))) / float(%4);
)";
    src +=
R"(
    return colour;
}
)";
    stringMultiReplace(src, {
        {"%1", name},
        {"%2", QString::number(bufferTextureLocation)},
        {"%3", bufferFormat.shaderSamplerType()},
        {"%4", QString::number(bufferFormat.scale())},
    });
    return src;
}

QString RenderManager::geometryShaderPart(const QString &name)
{
    QString src;
    src +=
R"(
//in layout(location = 0) vec2 pos;
in layout(location = 1) vec4 colour;

vec4 %1(const vec2 pos) {
    return colour;
}
)";
    stringMultiReplace(src, {
        {"%1", name},
    });
    return src;
}

QString RenderManager::dabShaderPart(const QString &name, const Dab::Type type, const int metric)
{
    const QMap<Dab::Type, QString> types = {
        { Dab::Type::Pixel,
R"(
float %1Brush(const vec2 pos) {
    return 0.0;
}
)"
        },
        { Dab::Type::Distance,
R"(
float %1Brush(const vec2 pos) {
    return %2(pos);
}
)"
        },
        { Dab::Type::Buffer,
R"(
float %1Brush(const vec2 pos) {
    return 0.0;
}
)"
        },
    };
    QString src;
    if (type == Dab::Type::Distance) {
        src += fileToString(":/shaders/distance.glsl");
    }
    src += types[type];
    src +=
R"(
layout(std140, binding = 0) uniform Data {
    mat3 matrix;
    vec4 colour;
    float hardness;
    float opacity;
} data;
uniform float %1Hardness = 0.0;
uniform float %1Opacity = 1.0;
uniform vec4 %1Colour = vec4(0.0, 0.25, 0.75, 1.0);
vec4 %1(const vec2 pos) {
    float weight;
    weight = clamp(1.0 - %1Brush(pos), 0.0, 1.0);
    weight = clamp(weight * (1.0 / (1.0 - %1Hardness)), 0.0, 1.0);
    weight *= %1Opacity;
    return vec4(%1Colour.rgb, %1Colour.a * weight);
}
)";
    stringMultiReplace(src, {
        {"%1", name},
        {"%2", distanceMetrics[metric].functionName},
    });
    return src;
}

QString RenderManager::colourSliderShaderPart(const QString &name, const ColourSpace colourSpace, const int component, const bool quantise, const GLint quantisePaletteTextureLocation, const Buffer::Format quantisePaletteFormat)
{
    QString src;
    src += fileToString(":/shaders/thirdparty/ColorSpaces.inc.glsl");
    if (quantise) {
        src += fileToString(":/shaders/quantise.glsl");
        src += paletteShaderPart("quantise", quantisePaletteTextureLocation, quantisePaletteFormat);
    }
    src +=
R"(
layout(std140, binding = 0) uniform Data {
    vec4 colour;
} data;
vec4 %1Colour = data.colour;
vec4 %1(const vec2 pos) {
)";
    if (component == colourSpaceInfo[colourSpace].componentCount) src +=
R"(
    const float alpha = pos.x;
    vec3 rgb = %1Colour.rgb;
)";
    else if (colourSpace != ColourSpace::RGB) src +=
R"(
    const float alpha = %1Colour.a;
    vec3 %2 = rgb_to_%2(%1Colour.rgb);
    %2[%3] = pos.x;
    vec3 rgb = %2_to_rgb(%2);
)";
    else src +=
R"(
    const float alpha = %1Colour.a;
    vec3 rgb = %1Colour.rgb;
    rgb[%3] = pos.x;
)";
    if (quantise) src +=
R"(
    rgb = quantisePalette(quantise(quantisePaletteTexture, %4, vec4(rgb, 1.0))).rgb;
)";
    src +=
R"(
    return vec4(rgb, alpha);
}
)";
    stringMultiReplace(src, {
        {"%1", name},
        {"%2", colourSpaceInfo[colourSpace].funcName},
        {"%3", QString::number(component)},
        {"%4", QString::number(quantise ? quantisePaletteFormat.scale() : 0)},
    });
    return src;
}

QString RenderManager::colourPlaneShaderPart(const QString &name, const ColourSpace colourSpace, const int componentX, const int componentY, const bool quantise)
{
    QString src;
    src += fileToString(":/shaders/thirdparty/ColorSpaces.inc.glsl");
    src +=
R"(
layout(std140, binding = 0) uniform Data {
    vec4 colour;
} data;
vec4 %1Colour = data.colour;
vec4 %1(const vec2 pos) {
)";
    if (componentX == colourSpaceInfo[colourSpace].componentCount) src +=
R"(
    const float alpha = pos.x;
    const vec3 rgb = %1Colour.rgb;
)";
    else if (colourSpace != ColourSpace::RGB) src +=
R"(
    const float alpha = %1Colour.a;
    vec3 %2 = rgb_to_%2(%1Colour.rgb);
    %2[%3] = pos.x;
    const vec3 rgb = %2_to_rgb(%2);
)";
    else src +=
R"(
    const float alpha = %1Colour.a;
    vec3 rgb = %1Colour.rgb;
    rgb[%3] = pos.x;
)";
    src +=
R"(
    return vec4(rgb, alpha);
}
)";
    stringMultiReplace(src, {
        {"%1", name},
        {"%2", colourSpaceInfo[colourSpace].funcName},
        {"%3", QString::number(componentX)},
        {"%4", QString::number(componentY)},
    });
    return src;
}

QString RenderManager::fragmentMainShaderPart(const Buffer::Format format, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat, const int blendMode, const int composeMode)
{
    QString src;
    src += fileToString(":/shaders/compositing.glsl");
    src += fileToString(":/shaders/blending.glsl");
    if (indexed && paletteFormat.isValid()) src += fileToString(":/shaders/quantise.glsl");
    src +=
R"(
in layout(location = 0) vec2 pos;

out %1 fragment;

void main(void) {
    const vec4 destColour = dest(gl_FragCoord.xy);
    const vec4 srcColour = src(pos);
    const vec4 blended = vec4(%4(destColour.rgb, srcColour.rgb), srcColour.a);
    const vec4 fragmentColour = unpremultiply(%5(destColour, blended));
)";
    if (indexed && paletteFormat.isValid()) src +=
R"(
    fragment = %1(quantise(destPaletteTexture, %3, fragmentColour));
)";
    else if (indexed && !paletteFormat.isValid()) src +=
R"(
    fragment = %1((fragmentColour.r + fragmentColour.g + fragmentColour.b) / 3.0 * %2);
)";
    else src +=
R"(
    fragment = %1(fragmentColour * %2);
)";
    src +=
R"(
}
)";
    stringMultiReplace(src, {
        {"%1", format.shaderValueType()},
        {"%2", QString::number(format.scale())},
        {"%3", QString::number(indexed && paletteFormat.isValid() ? paletteFormat.scale() : 1.0)},
        {"%4", RenderManager::blendModes[blendMode].functionName},
        {"%5", RenderManager::composeModes[composeMode].functionName},
    });
    return src;
}

QString RenderManager::widgetOutputShaderPart()
{
    QString src;
    src +=
R"(
in layout(location = 0) vec2 pos;

out vec4 fragment;

void main(void) {
    fragment = premultiply(src(pos));
}
)";
    return src;
}

} // namespace GfxPaint
