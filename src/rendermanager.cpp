#include "rendermanager.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <cstring>
#include <iostream>

#include "simplecpp/simplecpp.h"
#include "utils.h"
#include "application.h"
#include "renderedwidget.h"

namespace GfxPaint {

const Mat4 RenderManager::unitToClipTransform = ([](){
        static const GLfloat data[] = {
            2.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 1.0f,
        };
        Mat4 matrix;
        std::memcpy(matrix.data(), data, sizeof(data));
        return matrix;
    })();

const Mat4 RenderManager::clipToUnitTransform(RenderManager::unitToClipTransform.inverted());

const Mat4 RenderManager::flipTransform = ([](){
    static const GLfloat data[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    Mat4 matrix;
    std::memcpy(matrix.data(), data, sizeof(data));
    return matrix;
    })();

const std::map<ColourSpaceConversion, QString> RenderManager::colourSpaceConversionShaderFunctionNames = {
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
const QString RenderManager::shadersPath = ":/shaders";

RenderManager::RenderManager() :
    OpenGL(),
    surface(), context(),
    logger(),
    vao(),
    models(), programManager(), programs(),
    includeSources{}
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
    bool createContextSuccess = context.create();
//    bool createContextSuccess = false;
    // If OpenGL ES context creation not successfull try to fall back to comparable desktop OpenGL context
    if (!createContextSuccess) {
        surface.destroy();
        surface.setFormat(formatGL);
        surface.create();
        context.setFormat(surface.format());
        createContextSuccess = context.create();
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

#ifdef QT_DEBUG
    QObject::connect(&logger, &QOpenGLDebugLogger::messageLogged, [](const QOpenGLDebugMessage & debugMessage) {
        qDebug() << "OpenGL:" << debugMessage.message();
    });
    logger.initialize();
    logger.disableMessages(QOpenGLDebugMessage::AnySource, QOpenGLDebugMessage::AnyType, QOpenGLDebugMessage::NotificationSeverity);
    logger.disableMessages(QOpenGLDebugMessage::AnySource, QOpenGLDebugMessage::PerformanceType, QOpenGLDebugMessage::AnySeverity);
    logger.startLogging();
#endif

    OpenGL::initializeOpenGLFunctions();

    vao.create();
    vao.bind();

    models["clipQuad"] = new Model(GL_TRIANGLE_STRIP, {2,}, {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
        }, {
            0, 1, 2, 3,
        }, {4,});

    models["unitQuad"] = new Model(GL_TRIANGLE_STRIP, {2,}, {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
        }, {
            0, 1, 2, 3,
        }, {4,});

    models["sliderMarker"] = new Model(GL_TRIANGLES, {2, 4,}, {
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
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

    models["paletteMouseMarker"] = new Model(GL_TRIANGLES, {2, 4,}, {
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
            0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        }, {
            0, 1, 2,
            3, 4, 5,
        }, {3, 3,});

    std::vector<QString> includes = {};
    QDirIterator it(shadersPath, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QFileInfo fileInfo = it.fileInfo();
        const QString &extension = fileInfo.suffix();
        if (!fileInfo.isDir() && QString::compare(extension, "glsl", Qt::CaseInsensitive) == 0) {
            includes.push_back(fileInfo.filePath());
        }
        it.next();
    }
    addGlslIncludes(includes);

    programs["marker"] = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
}

RenderManager::~RenderManager()
{
    {
        ContextBinder contextBinder(&context, &surface);

        models.clear();
        programs.clear();

        logger.stopLogging();

        vao.destroy();
    }
}

bool RenderManager::isOpenGLES()
{
    return qApp->renderManager.surface.format().renderableType() == QSurfaceFormat::OpenGLES;
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

QString RenderManager::glslVersionString()
{
    QString src;
    const auto &format = qApp->renderManager.surface.format();
    src += "#version " + QString::number(format.majorVersion()) + QString::number(format.minorVersion()) + "0" + (isOpenGLES() ? " es" : " core") + "\n";
    return src;
}

GLuint RenderManager::bufferAddDepthStencilAttachment(Buffer *const buffer)
{
    ContextBinder contextBinder(&context, &surface);
    FramebufferBinder framebufferBinder(GL_FRAMEBUFFER, buffer->framebuffer());
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, buffer->width(), buffer->height(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
    return texture;
}

void RenderManager::bufferRemoveDepthStencilAttachment(Buffer *const buffer, const GLuint texture)
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    FramebufferBinder framebufferBinder(GL_FRAMEBUFFER, buffer->framebuffer());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    glDeleteTextures(1, &texture);
}

void RenderManager::addGlslIncludes(const std::vector<QString> &includes)
{
    const QDir shadersDir(shadersPath);
    for (const QString &path : includes) {
        const QString &include = shadersDir.relativeFilePath(path);
        const QString &src = resourceShaderPart(include);
        includeSources[include.toStdString()] = src.toStdString();
    }
}

QString RenderManager::preprocessGlsl(const QString &inputSrc, const QString &filename, const std::map<QString, QString> &defines = {})
{
    simplecpp::DUI dui;
    for (const auto &[name, value] : defines) {
        QString str = name;
        if (!value.isEmpty()) {
            str += "=" + value;
        }
        dui.defines.push_back(str.toStdString());
    }
    simplecpp::OutputList outputList;
    std::vector<std::string> files;
    std::istringstream inputStream(inputSrc.toStdString());
    simplecpp::TokenList rawTokens(inputStream, files, filename.toStdString(), &outputList);
    rawTokens.removeComments();
    std::map<std::string, simplecpp::TokenList*> includeTokenLists = simplecpp::load(rawTokens, files, dui, &outputList);
    for (const auto &[path, src] : includeSources) {
        std::istringstream f(src);
        simplecpp::TokenList rawtokens(f,files,path,&outputList);
        includeTokenLists[path] = new simplecpp::TokenList(rawtokens);
    }
    for (auto &[path, tokenList] : includeTokenLists) {
        tokenList->removeComments();
    }
    for (const auto &file : files) qDebug() << "FILE:" << QString::fromStdString(file);//////////////////////////
    qDebug() << "INCLUDES:" << includeTokenLists.size();//////////////////////////
    simplecpp::TokenList outputTokens(files);
    simplecpp::preprocess(outputTokens, rawTokens, files, includeTokenLists, dui, &outputList);
    QString preprocessedSrc = QString::fromStdString(outputTokens.stringify());
    simplecpp::cleanup(includeTokenLists);
    return preprocessedSrc;

    /*    QString src;
    for (const auto &[key, value] : defines) {
        src += "#define " + key + " " + value + "\n";
    }
    qDebug().noquote() << src;////////////////////////////////////////
    src += inputSrc;

    tcpp::StringInputStream input(src.toStdString());
    tcpp::Lexer lexer(input);

    bool result = true;

    std::set<tcpp::StringInputStream *> streams;
    const auto &system = systemIncludeSources;
    const auto &local = localIncludeSources;

    tcpp::Preprocessor preprocessor(lexer,
        [&result](const tcpp::TErrorInfo &errorInfo) {
            qDebug() << "GLSL preprocessor error:" << QString::fromStdString(ErrorTypeToString(errorInfo.mType));
            result = false;
        },
        [&streams, &system, &local](const std::string &path, const bool isSystemInclude) {
            const auto &sources = isSystemInclude ? system : local;
            tcpp::StringInputStream *stream;
            if (sources.contains(path)) {
                stream = new tcpp::StringInputStream(sources.at(path));
            }
            else {
                qDebug() << "GLSL preprocessor error:" << "include not found" << QString::fromStdString(path);
                stream = new tcpp::StringInputStream("");
            }
            streams.insert(stream);
            return stream;
        });

    const std::string &output = preprocessor.Process();

    streams.clear();

    return QString::fromStdString(output);*/
}

QString RenderManager::headerShaderPart()
{
    QString src;
    src += qApp->renderManager.glslVersionString();
    if (isOpenGLES()) {
        src += resourceShaderPart("precision.glsl");
    }
    src += resourceShaderPart("types.glsl");
    src += resourceShaderPart("util.glsl");
    return src;
}

QString RenderManager::resourceShaderPart(const QString &filename)
{
    QString src;
    src += fileToString(":/shaders/" + filename);
    return src;
}

QString RenderManager::colourSpaceShaderPart()
{
    QString src;
    src += resourceShaderPart("ColorSpaces.inc.glsl");
    for (const auto &[key, value] : colourSpaceInfo) {
        QString str = R"(
vec3 $COLOURSPACE_to_$COLOURSPACE(vec3 rgb) {
    return rgb;
}
)";
        stringMultiReplace(str, {
            {"$COLOURSPACE", value.funcName},
        });
        src += str;
    }
    return src;
}

QString RenderManager::attributelessShaderPart(const AttributelessModel model)
{
    const std::map<AttributelessModel, QString> models = {
        { AttributelessModel::SingleVertex, R"(
const vec2 vertices[1] = vec2[](
    vec2(0.0, 0.0)
);
)"
        },
        // Clip-space quad in triangle strip order
        { AttributelessModel::ClipQuad, R"(
const vec2 vertices[4] = vec2[](
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0)
);
)"
        },
        // Unit quad in triangle strip order
        { AttributelessModel::UnitQuad, R"(
const vec2 vertices[4] = vec2[](
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);
)"
        },
    };
    QString src;
    src += models.at(model);
    return src;
}

QString RenderManager::vertexMainShaderPart()
{
    QString src;
    src += R"(
uniform mat4 transform;
uniform mat4 object;

out layout(location = 0) vec2 pos;

void main(void) {
    vec2 vertexPos = vertices[gl_VertexID];
    pos = (object * vec4(vertexPos, 0.0, 1.0)).xy;
    gl_Position = (transform * object) * vec4(vertexPos, 0.0, 1.0);
}
)";
    return src;
}

QString RenderManager::patternShaderPart(const QString &name, const Pattern pattern)
{
    QString src;
    switch (pattern) {
    case Pattern::Checkers: {
    src += R"(
const float base = 7.0 / 16.0;
const float offset = 1.0 / 16.0;
const float light = base + offset;
const float dark = base - offset;

vec4 lightColour = vec4(vec3(light), 1.0);
vec4 darkColour = vec4(vec3(dark), 1.0);

Colour $NAME(const vec2 pos) {
    bool alternate = !((mod(pos.x, 2.0) >= 1.0) == (mod(pos.y, 2.0) >= 1.0));
    return Colour(alternate ? lightColour : darkColour, INDEX_INVALID);
}
)";
    } break;
    case Pattern::Bricks: {
    } break;
    }
    stringMultiReplace(src, {
        {"$NAME", name},
    });
    return src;
}

QString RenderManager::paletteShaderPart(const QString &name, const GLint paletteTextureLocation, const Buffer::Format paletteFormat)
{
    QString src;
    src += R"(
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
    src += R"(
uniform layout(location = $TEXTURE_LOCATION) $SAMPLER_TYPE $NAMETexture;

layout(std140, binding = $UNIFORM_BLOCK_BINDING) uniform $NAMEUniformData {
    mat4 matrix;
    Colour transparent;
} $NAMEData;

Colour $NAME(const vec2 pos) {
    Colour colour = COLOUR_INVALID;
//    Colour transparent = $NAMEData.transparent;
)";
    if (indexed && paletteFormat.isValid()) src += R"(
    colour.index = texelFetch($NAMETexture, ivec2(floor(pos)), 0).x;
//    colour.rgba = (colour.index == transparent.index ? vec4(0.0) : $NAMEPalette(colour.index));
    colour.rgba = $NAMEPalette(colour.index);
)";
    else if (indexed && !paletteFormat.isValid()) src += R"(
    float grey = toUnit(texelFetch($NAMETexture, ivec2(floor(pos)), 0).x, $SCALAR_VALUE_TYPE($FORMAT_SCALE));
    colour.rgba = (colour.index == transparent.index ? vec4(0.0) : vec4(vec3(grey), 1.0));
)";
    else src += R"(
    vec4 texelRgba = toUnit(texelFetch($NAMETexture, ivec2(floor(pos)), 0), $SCALAR_VALUE_TYPE($FORMAT_SCALE));
//    colour.rgba = (texelRgba == transparent.rgba ? vec4(0.0) : texelRgba);
//    if (transparent.rgba != RGBA_INVALID) {
//        colour.rgba = (texelRgba == transparent.rgba ? vec4(0.0) : texelRgba);
//    }
//    else {
        colour.rgba =  texelRgba;
//    }
)";
    src += R"(
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

QString RenderManager::colourPlaneShaderPart(const QString &name, const ColourSpace colourSpace, const bool useXAxis, const bool useYAxis, const bool quantise, const GLint quantisePaletteTextureLocation, const Buffer::Format quantisePaletteFormat)
{
    QString src;
    src += colourSpaceShaderPart();
    if (quantise) {
        src += resourceShaderPart("palette.glsl");
        src += paletteShaderPart("quantise", quantisePaletteTextureLocation, quantisePaletteFormat);
    }
    src += R"(
layout(std140, binding = 0) uniform Data {
    Colour colour;
    ivec2 components;
} data;

vec4 colourPlane(const vec4 colour, const ivec2 components, const vec2 pos) {
    vec4 col = colour;
)";
    if (useXAxis) src += R"(
    col[components[0]] = pos.x;
)";
    if (useYAxis) src += R"(
    col[components[1]] = pos.y;
)";
    src += R"(
    return col;
}

Colour $NAME(const vec2 pos) {
    uint index = INDEX_INVALID;
    vec4 col = data.colour.rgba;
    col = vec4(rgb_to_$COLOURSPACE(col.rgb), col.a);
    col = colourPlane(col, data.components, pos);
    col = vec4($COLOURSPACE_to_rgb(col.rgb), col.a);
)";
    if (quantise && quantisePaletteFormat.isValid()) src += R"(
    index = quantiseBruteForce(quantisePaletteTexture, uint($PALETTE_FORMAT_SCALE), vec4(col.rgb, 1.0), 0.5, INDEX_INVALID);
    col = vec4(quantisePalette(index).rgb, col.a);
)";
    src += R"(
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
    src += R"(
uniform ivec2 cells;

ivec2 posCell(const ivec2 cells, const vec2 pos) {
    return ivec2(floor(vec2(cells) * pos));
}

Index cellIndex(const ivec2 cells, const int size, const vec2 pos) {
    ivec2 cell = posCell(cells, pos);
    int index = cell.x + cell.y * cells.x;
    if (cell.x >= 0 && cell.x < cells.x &&
        cell.y >= 0 && cell.y < cells.y &&
        index >= 0 && index < size)
        return Index(index);
    else return INDEX_INVALID;
}

Colour $NAME(const vec2 pos) {
    Index index = cellIndex(cells, int(paletteSize($NAMEPalettePaletteTexture)), pos);
    if (index != INDEX_INVALID)
        return Colour(vec4($NAMEPalettePalette(Index(index))), Index(index));
    else return COLOUR_INVALID;
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
    });
    return src;
}

QString RenderManager::standardInputFragmentShaderPart(const QString &name)
{
    QString src;
    src += R"(
in layout(location = 0) vec2 pos;

Colour src(void) {
    return $NAME(pos);
}
)";
    stringMultiReplace(src, {
        {"$NAME", name},
    });
    return src;
}

QString RenderManager::standardFragmentMainShaderPart(const Buffer::Format format, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat, const int blendMode, const int composeMode)
{
    QString src;
    src += resourceShaderPart("compositing.glsl");
    src += resourceShaderPart("blending.glsl");
    if (indexed && paletteFormat.isValid()) src += resourceShaderPart("palette.glsl");
    src += R"(
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
    Colour srcColour = src();
    vec4 blended = blend(destColour.rgba, srcColour.rgba);
    vec4 composed = compose(destColour.rgba, blended);
)";
    if (indexed && paletteFormat.isValid()) src += R"(
    fragment = $VALUE_TYPE(quantiseBruteForce(destPaletteTexture, uint($PALETTE_FORMAT_SCALE), composed, 0.5, destData.transparent.index));
)";
    else if (indexed && !paletteFormat.isValid()) src += R"(
    fragment = $VALUE_TYPE(fromUnit((composed.r + composed.g + composed.b) / 3.0, $SCALAR_VALUE_TYPE($FORMAT_SCALE)));
)";
    else src += R"(
    fragment = $VALUE_TYPE(fromUnit(composed, $SCALAR_VALUE_TYPE($FORMAT_SCALE)));
)";
    src += R"(
}
)";
    stringMultiReplace(src, {
        {"$VALUE_TYPE", format.shaderValueType()},
        {"$FORMAT_SCALE", QString::number(format.scale())},
        {"$PALETTE_FORMAT_SCALE", QString::number(indexed && paletteFormat.isValid() ? paletteFormat.scale() : 1.0)},
        {"$BLEND_MODE", blendModes[blendMode].functionName},
        {"$COMPOSE_MODE", composeModes[composeMode].functionName},
        {"$SCALAR_VALUE_TYPE", format.shaderScalarValueType()},
    });
    return src;
}

QString RenderManager::widgetFragmentMainShaderPart()
{
    QString src;
    src += R"(
out layout(location = 0) vec4 fragment;

void main(void) {
    fragment = premultiply(src().rgba);
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
