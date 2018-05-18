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

    ProgramManager programManager;

    static QString headerShaderPart();
    static QString modelShaderPart(const Model model);
    static QString vertexMainShaderPart();
    static QString patternShaderPart(const QString &name, const Pattern pattern);
    static QString dabShaderPart(const QString &name, const Dab::Type type, const Metric metric);
    static QString bufferShaderPart(const QString &name, const GLint bufferTextureLocation, const Buffer::Format bufferFormat, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat);
    static QString colourSliderShaderPart(const QString &name, const ColourSpace colourSpace, const int component);
    static QString blenderShaderPart(const Blender blender);
    static QString metricShaderPart(const Metric metric);
    static QString fragmentMainShaderPart(const Buffer::Format format, const bool indexed);
    static QString widgetOutputShaderPart();
};

} // namespace GfxPaint

#endif // RENDERMANAGER_H
