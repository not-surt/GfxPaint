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
    return src.arg(QString::number(OPENGL_MAJOR_VERSION) + QString::number(OPENGL_MINOR_VERSION) + "0");
}

QString RenderManager::modelShaderPart(const Model model)
{
    const QMap<Model, QString> models = {
        { Model::SingleVertex,
R"(
const vec2 vertices[1] = vec2[](
    vec2(0.0, 0.0)
);
)"
        },
        { Model::ClipQuad,
R"(
const vec2 vertices[4] = vec2[](
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);
)"
        },
        { Model::UnitQuad,
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
    src = src.arg(name);
    return src;
}

QString RenderManager::bufferShaderPart(const QString &name, const GLint bufferTextureLocation, const Buffer::Format bufferFormat, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat)
{
    QString src;
    src +=
QString(R"(
uniform layout(location = %2) %3 %1Texture;
)").arg(name)
            .arg(bufferTextureLocation)
            .arg(bufferFormat.shaderSamplerType());
    if (indexed && paletteFormat.componentType != Buffer::Format::ComponentType::Invalid) src +=
QString(R"(
uniform layout(location = %2) %3 %1Palette;
)").arg(name)
            .arg(paletteTextureLocation)
            .arg(paletteFormat.shaderSamplerType());
    src +=
QString(R"(
vec4 %1(const vec2 pos) {
)").arg(name);
    if (indexed && paletteFormat.componentType != Buffer::Format::ComponentType::Invalid) src +=
QString(R"(
    const uint index = texelFetch(%1Texture, ivec2(floor(pos))).x;
    const vec4 colour = vec4(texelFetch(%1Palette, ivec2(index, 0))) / %2;
    return colour;
)").arg(name)
            .arg(paletteFormat.scale());
    else src +=
QString(R"(
    const vec4 colour = vec4(texelFetch(%1Texture, ivec2(floor(pos)))) / %2;
    return colour;
)").arg(name)
            .arg(bufferFormat.scale());
    src +=
R"(
}
)";
    return src;
}

QString RenderManager::dabShaderPart(const QString &name, const Dab::Type type)
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
    return metric(pos);
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
    src += types[type];
    src +=
R"(
layout(std140, binding = 0) uniform Data {
    mat3 matrix;
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
    return src.arg(name);
}

QString RenderManager::colourSliderShaderPart(const QString &name, const ColourSpace colourSpace, const int component)
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
    if (component == 3) src +=
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
    //return src.arg(name).arg(colourSpaceNames[colourSpace]).arg(component);
    stringMultiReplace(src, {
        {"%1", name},
        {"%2", colourSpaceInfo[colourSpace].funcName},
        {"%3", QString::number(component)},
    });
    return src;
}

QString RenderManager::blenderShaderPart(const Blender blender) {
    const QMap<Blender, QString> blenders = {
        { Blender::Alpha,
R"(
vec4 blend(vec4 src, vec4 dest) {
    src = premultiply(src);
    dest = premultiply(dest);
    const float a = src.a + dest.a * (1.0 - src.a);
    const vec3 rgb = src.rgb + dest.rgb * (1.0 - src.a);
    return unpremultiply(vec4(rgb, a));
//      return vec4(BlendOpacity(dest.rgb, src.rgb, BlendNormal, src.a), src.a + (1.0 - src.a) * dest.a);
//      return vec4(vec3(1, 0, 0), src.a + (1.0 - src.a) * dest.a);
}
)"
        },
        { Blender::Erase,
R"(
vec4 blend(const vec4 src, const vec4 dest) {
    //return vec4(dest.rgb, dest.a * (1.0 - src.a));
    return vec4(dest.rgb, dest.a - src.a);
}
)"
        },
        { Blender::Replace,
R"(
vec4 blend(const vec4 src, const vec4 dest) {
    return src;
}
)"
        },
        { Blender::Add,
R"(
vec4 blend(const vec4 src, const vec4 dest) {
    return vec4(dest.rgb + src.rgb * src.a, dest.a);
}
)"
        },
        { Blender::Subtract,
R"(
vec4 blend(const vec4 src, const vec4 dest) {
    return vec4(dest.rgb - src.rgb * src.a, dest.a);
}
)"
        },
        { Blender::Multiply,
R"(
vec4 blend(const vec4 src, const vec4 dest) {
    return vec4(dest.rgb * src.rgb * src.a, dest.a);
}
)"
        },
        { Blender::Divide,
R"(
vec4 blend(const vec4 src, const vec4 dest) {
    return vec4(dest.rgb / src.rgb * src.a, dest.a);
}
)"
        },
    };
    QString src;
    src += fileToString(":/shaders/blending.glsl");
    src += fileToString(":/shaders/thirdparty/PhotoshopMathFP.glsl");
    src += blenders[blender];
    return src;
}

QString RenderManager::metricShaderPart(const Metric metric)
{
    const QMap<Metric, QString> metrics = {
        { Metric::Euclidean,
R"(
float metric(const vec2 pos) {
    return sqrt(pow(pos.x, 2) + pow(pos.y, 2));
}
)"
        },
        { Metric::Manhattan,
R"(
float metric(const vec2 pos) {
    return abs(pos.x) + abs(pos.y);
}
)"
        },
        { Metric::Chebyshev,
R"(
float metric(const vec2 pos) {
    return max(abs(pos.x), abs(pos.y));
}
)"
        },
        { Metric::Minimum,
R"(
float metric(const vec2 pos) {
    return min(abs(pos.x), abs(pos.y));
}
)"
        },
        { Metric::Octagonal,
R"(
float metric(const vec2 pos) {
      return (1007.0/1024.0) * max(abs(pos.x), abs(pos.y)) + (441.0/1024.0) * min(abs(pos.x), abs(pos.y));
}
)"
        },
    };
    QString src;
    src += metrics[metric];
    return src;
}

QString RenderManager::fragmentMainShaderPart(const Buffer::Format format, const bool indexed, const Buffer::Format paletteFormat)
{
    QString src;
    src += fileToString(":/shaders/compositing.glsl");
    src +=
R"(
in layout(location = 0) vec2 pos;

out %1 fragment;

void main(void) {
    const vec4 destColour = dest(gl_FragCoord.xy);
    const vec4 srcColour = src(pos);
    const vec4 blended = vec4(blendNormal(destColour.rgb, srcColour.rgb), srcColour.a);
    const vec4 fragmentColour = porterDuffSrcOver(destColour, blended);
)";
    if (indexed) src +=
R"(
    uint nearestIndex = 0;
    float nearestDistance = pow(%2 + 1, 3);
    for (uint index = 0; index < uint(textureSize(destPalette, 0).x); ++index) {
        const vec4 paletteColour = vec4(texelFetch(destPalette, ivec2(index, 0))) / float(%3);
        const float colourDistance = distance(fragmentColour.rgb, paletteColour.rgb);
        if (colourDistance < nearestDistance) {
            nearestIndex = index;
            nearestDistance = colourDistance;
        }
    }
    fragment = nearestIndex;
    //fragment = uint(gl_FragCoord.x) % uint(textureSize(destPalette, 0).x);
)";
    else src +=
R"(
    %3 + %3;//////////////////
    fragment = %1(fragmentColour * %2);
)";
    src +=
R"(
}
)";
    return src.arg(format.shaderValueType()).arg(format.scale()).arg(indexed && paletteFormat.isValid() ? paletteFormat.scale() : 1.0);
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
