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
    struct ProgramConstructionState {
        GLint uniformBlockBinding = 0;
        GLint uniformLocation = 0;
        GLint imageBinding = 0;
        GLint attributeLocation = 0;

        GLint nextUniformBlockBinding() {
            return uniformBlockBinding++;
        }
        GLint nextUniformLocation() {
            return uniformLocation++;
        }
        GLint nextImageBinding() {
            return imageBinding++;
        }
        GLint nextAttributeLocation() {
            return attributeLocation++;
        }
    };

    struct DistanceMetricInfo {
        QString label;
        QString functionName;
    };
    struct BlendModeInfo {
        QString label;
        QString functionName;
    };
    struct ComposeModeInfo {
        QString label;
        QString functionName;
    };

    static constexpr std::tuple<int, int> openGLVersion = {4, 3};
    static constexpr std::tuple<int, int> openGLESVersion = {3, 2};

    static const Mat4 unitToClipTransform;
    static const Mat4 clipToUnitTransform;
    static const Mat4 flipTransform;
    static const std::map<ColourSpaceConversion, QString> colourSpaceConversionShaderFunctionNames;
    static const QList<DistanceMetricInfo> distanceMetrics;
    static const QList<BlendModeInfo> blendModes;
    static const QList<ComposeModeInfo> composeModes;
    static const int composeModeDefault;

    QOffscreenSurface surface;
    QOpenGLContext context;
    QOpenGLDebugLogger logger;
    QOpenGLVertexArrayObject vao;

    std::map<QString, Model *> models;
    ProgramManager programManager;
    std::map<QString, Program *> programs;

    explicit RenderManager();
    virtual ~RenderManager();

    static QSurfaceFormat defaultFormat();

    QString glslVersionString() const;
    QString glslPrecisionString() const;

    static QString headerShaderPart();
    static QString resourceShaderPart(const QString &filename);
    static QString colourSpaceShaderPart();
    static QString attributelessShaderPart(const AttributelessModel model);
    static QString modelVertexMainShaderPart();
    static QString vertexMainShaderPart();
    static QString patternShaderPart(const QString &name, const Pattern pattern);
    static QString brushDabShaderPart(const QString &name, const Brush::Dab::Type type, const int metric);
    static QString paletteShaderPart(const QString &name, const GLint paletteTextureLocation, const Buffer::Format paletteFormat);
    static QString bufferShaderPart(const QString &name, const GLint uniformBlockBinding, const GLint bufferTextureLocation, const Buffer::Format bufferFormat, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat);
    static QString standardInputFragmentShaderPart(const QString &name);
    static QString modelFragmentShaderPart(const QString &name);
    static QString colourPlaneShaderPart(const QString &name, const ColourSpace colourSpace, const bool useXAxis, const bool useYAxis, const bool quantise, const GLint quantisePaletteTextureLocation, const Buffer::Format quantisePaletteFormat);
    static QString colourPaletteShaderPart(const QString &name);
    static QString standardFragmentMainShaderPart(const Buffer::Format format, const bool indexed, const GLint paletteTextureLocation, const Buffer::Format paletteFormat, const int blendMode, const int composeMode);
    static QString widgetFragmentMainShaderPart();

    void bindBufferShaderPart(QOpenGLShaderProgram &program, const QString &name, const GLint bufferTextureLocation, const Buffer *const buffer);
    void bindIndexedBufferShaderPart(QOpenGLShaderProgram &program, const QString &name, const GLint bufferTextureLocation, const Buffer *const buffer, const bool indexed, const GLint paletteTextureLocation, const Buffer *const palette);
};

} // namespace GfxPaint

#endif // RENDERMANAGER_H
