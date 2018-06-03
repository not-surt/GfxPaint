#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include "opengl.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "buffer.h"
#include "brush.h"
#include "types.h"
#include "program.h"

namespace GfxPaint {

class RenderManager : protected OpenGL
{
public:
    static const QMap<ColourSpaceConversion, QString> colourSpaceConversionShaderFunctionNames;

    explicit RenderManager();
    virtual ~RenderManager();

    QOffscreenSurface surface;
    QOpenGLContext context;

    QOpenGLDebugLogger logger;

    QOpenGLVertexArrayObject vao;

    static const QTransform unitToClipTransform;
    static const QTransform clipToUnitTransform;
    static const QTransform flipTransform;

    struct DistanceMetricInfo {
        QString label;
        QString functionName;
    };
    static const QList<DistanceMetricInfo> distanceMetrics;
    struct intInfo {
        QString label;
        QString functionName;
    };
    static const QList<intInfo> blendModes;
    struct ComposeModeInfo {
        QString label;
        QString functionName;
    };
    static const QList<ComposeModeInfo> composeModes;
    static const int composeModeDefault;

    ProgramManager programManager;

    static QString headerShaderPart();
    static QString attributelessShaderPart(const Model model);
    static QString geometryVertexShaderPart();
    static QString vertexMainShaderPart();
    static QString patternShaderPart(const QString &name, const Pattern pattern);
    static QString dabShaderPart(const QString &name, const Dab::Type type, const int metric);
    static QString paletteShaderPart(const QString &name, const GLint paletteTextureLocation, const Buffer::Format paletteFormat);
    static QString bufferShaderPart(const QString &name, const GLint bufferTextureLocation, const Buffer::Format bufferFormat, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat);
    static QString geometryShaderPart(const QString &name);
    static QString colourSliderShaderPart(const QString &name, const ColourSpace colourSpace, const int component, const bool quantise, const GLint quantisePaletteTextureLocation, const Buffer::Format quantisePaletteFormat);
    static QString colourPlaneShaderPart(const QString &name, const ColourSpace colourSpace, const int componentX, const int componentY, const bool quantise);
    static QString fragmentMainShaderPart(const Buffer::Format format, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat, const int blendMode, const int composeMode);
    static QString widgetOutputShaderPart();
};

} // namespace GfxPaint

#endif // RENDERMANAGER_H
