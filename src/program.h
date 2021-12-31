#ifndef PROGRAM_H
#define PROGRAM_H

#include "opengl.h"

#include <typeindex>
#include <QOpenGLShaderProgram>
#include <deque>

#include "types.h"
#include "buffer.h"
#include "stroke.h"
#include "brush.h"
#include "model.h"
#include "utils.h"

namespace GfxPaint {

const std::array programStages{
    QOpenGLShader::Geometry, QOpenGLShader::Vertex, QOpenGLShader::Fragment, QOpenGLShader::Compute,
};

struct SimpleProgramState {
    GLint attributeLocation = 0;
    GLint uniformLocation = 0;
    GLint bindingLocation = 0;

    GLint nextAttributeLocation() {
        return attributeLocation++;
    }
    GLint nextUniformLocation() {
        return uniformLocation++;
    }
    GLint nextBindingLocation() {
        return bindingLocation++;
    }
};

struct SimpleProgramComponent {
    virtual ~SimpleProgramComponent() {}
    virtual QOpenGLShader::ShaderType stages() const = 0;
    virtual QString definitions(SimpleProgramState &state) const { return QString(); }
    virtual QString main(SimpleProgramState &state) const { return QString(); }
    virtual void bind() const {}
};

struct VertexPositionProgramComponent {
    virtual ~VertexPositionProgramComponent() {}
    virtual QOpenGLShader::ShaderType stages() { return QOpenGLShader::Vertex; }
    virtual QString definitions(SimpleProgramState &state) const {
        return QString("in layout(location = %1) vec2 vertexPos;\n").arg(state.nextAttributeLocation());
    }
    virtual QString main(SimpleProgramState &state) const { return QString(); }
};

struct SimpleProgram {
    const QList<SimpleProgramComponent *> components;

    SimpleProgram(const QList<SimpleProgramComponent *> &components) :
        components(components)
    {
        QOpenGLShaderProgram *program = new QOpenGLShaderProgram();
        QMap<QOpenGLShader::ShaderTypeBit, QString> definitions, main;
        SimpleProgramState state;
        for (const auto stage : programStages) {
            for (const auto component : components) {
                if (component->stages() & stage) {
                    definitions[stage] += component->definitions(state);
                    main[stage] += component->main(state);
                }
            }
            if (!main[stage].isEmpty()) {
                QString src;
                src += definitions[stage];
                src += "void main() {\n";
                src += main[stage];
                src += "}\n";
                program->addShaderFromSourceCode(stage, src);
            }
        }
        program->link();
    }
    virtual ~SimpleProgram() {
        qDeleteAll(components);
    }

    void bind() {
        for (const auto component : components) {
            component->bind();
        }
    }
};

struct ProgramComponent {
    struct UniformBlock {
        QOpenGLShader::ShaderType stages;
        GLuint binding;
        QString name;
    };
    struct Uniform {
        QOpenGLShader::ShaderType stages;
        GLuint location;
        QString name;
    };
    struct Storage {
        QOpenGLShader::ShaderType stages;
        GLuint binding;
        QString name;
    };
    struct Attribute {
        QOpenGLShader::ShaderType stages;
        GLuint location;
        QString name;
    };
    struct Output {
        QOpenGLShader::ShaderType stages;
        GLuint location;
        QString name;
    };
    struct Input {
        QOpenGLShader::ShaderType stages;
        GLuint location;
        QString name;
    };
    struct IncludeSrc {
        QOpenGLShader::ShaderType stages;
        QString path;
    };
    struct DefinitionsSrc {
        QOpenGLShader::ShaderType stages;
        QString src;
    };
    struct MainSrc {
        QOpenGLShader::ShaderType stages;
        QString src;
    };

    QList<Uniform> uniformBlocks;
    QList<Uniform> uniforms;
    QList<Storage> storages;
    QList<Attribute> attributes;
    QList<Output> outputs;
    QList<Input> inputs;
    QList<IncludeSrc> includeSrcs;
    QList<DefinitionsSrc> definitionsSrcs;
    QList<MainSrc> mainSrcs;
};

struct ComponentProgram {
    QString inputDefinition() {
        QString str = "layout(location = %1) in %2 %3;";
        stringMultiReplace(str, {
            {"%1", "location"},
            {"%2", "type"},
            {"%3", "name"},
        });
        return str;
    }
    QString outputDefinition() {
        QString str = "layout(location = %1) out %2 %3;";
        stringMultiReplace(str, {
            {"%1", "location"},
            {"%2", "type"},
            {"%3", "name"},
        });
        return str;
    }

    QList<ProgramComponent> components = {};

    QOpenGLShaderProgram *createProgram() const {
        QOpenGLShaderProgram *const program = new QOpenGLShaderProgram();

        for (const auto stage : programStages) {
            switch (stage) {
            case  QOpenGLShader::Geometry: {
            } break;
            case  QOpenGLShader::Vertex: {
            } break;
            case  QOpenGLShader::Fragment: {
            } break;
            case  QOpenGLShader::Compute: {
            } break;
            default: {
            }
            }
        }

        return program;
    }
};

class Program : protected OpenGL {
public:
    typedef std::pair<std::type_index, std::list<int>> Key;

    Program();
    Program(const Program &other) :
        OpenGL(true),
        key(other.key), m_program(nullptr)
    {}
    virtual ~Program();

    QOpenGLShaderProgram &program();

protected:
    void updateKey(const std::type_index type, const std::list<int> &values) {
        key.first = type;
        std::list<int> &list = key.second;
        list.insert(list.end(), values.begin(), values.end());
    }
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const = 0;
    QOpenGLShaderProgram *createProgram() const;

private:
    Key key;
    QOpenGLShaderProgram *m_program;
};

class ToolProgram : public Program {
public:
    ToolProgram() :
        Program(),
        storageBuffer(0)
    {
        updateKey(typeid(this), {});

        glGenBuffers(1, &storageBuffer);
    }
    ToolProgram(const ToolProgram &other) :
        Program(other),
        storageBuffer(0)
    {
        glGenBuffers(1, &storageBuffer);
    }
    virtual ~ToolProgram() override {
        glDeleteBuffers(1, &storageBuffer);
    }

protected:
    GLuint storageBuffer;
};

class RenderProgram : public Program {
public:
    RenderProgram(const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        Program(),
        destFormat(destFormat), destIndexed(destIndexed), destPaletteFormat(destPaletteFormat),
        blendMode(blendMode), composeMode(composeMode),
        uniformBuffer(0)
    {
        updateKey(typeid(this), {static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, static_cast<int>(destIndexed), static_cast<int>(destPaletteFormat.componentType), destPaletteFormat.componentSize, destPaletteFormat.componentCount, blendMode, composeMode});

        glGenBuffers(1, &uniformBuffer);
    }
    RenderProgram(const RenderProgram &other) :
        Program(other),
        destFormat(other.destFormat), destIndexed(other.destIndexed), destPaletteFormat(other.destPaletteFormat),
        blendMode(other.blendMode), composeMode(other.composeMode),
        uniformBuffer(0)
    {
        glGenBuffers(1, &uniformBuffer);
    }
    virtual ~RenderProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

protected:
    const Buffer::Format destFormat;
    const bool destIndexed;
    const Buffer::Format destPaletteFormat;
    const int blendMode;
    const int composeMode;

    GLuint uniformBuffer;
};

class RenderedWidgetProgram : public Program {
public:
    RenderedWidgetProgram(const Buffer::Format srcFormat, const bool srcIndexed, const Buffer::Format srcPaletteFormat) :
        Program(),
        srcFormat(srcFormat), srcIndexed(srcIndexed), srcPaletteFormat(srcPaletteFormat)
    {
        updateKey(typeid(this), {static_cast<int>(srcFormat.componentType), srcFormat.componentSize, srcFormat.componentCount, static_cast<int>(srcIndexed)});
    }
    RenderedWidgetProgram(const RenderedWidgetProgram &other) :
        Program(),
        srcFormat(other.srcFormat), srcIndexed(other.srcIndexed), srcPaletteFormat(other.srcPaletteFormat)
    {}

    void render(Buffer *const src, const Mat4 &worldToClip);

protected:
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Buffer::Format srcFormat;
    const bool srcIndexed;
    const Buffer::Format srcPaletteFormat;
};

class BufferProgram : public RenderProgram {
public:
    BufferProgram(const Buffer::Format srcFormat, const bool srcIndexed, const Buffer::Format srcPaletteFormat, const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        srcFormat(srcFormat), srcIndexed(srcIndexed), srcPaletteFormat(srcPaletteFormat)
    {
        updateKey(typeid(this), {static_cast<int>(srcFormat.componentType), srcFormat.componentSize, srcFormat.componentCount, static_cast<int>(srcIndexed), static_cast<int>(srcPaletteFormat.componentType), srcPaletteFormat.componentSize, srcPaletteFormat.componentCount});
    }
    BufferProgram(const BufferProgram &other) :
        RenderProgram(other),
        srcFormat(other.srcFormat), srcIndexed(other.srcIndexed), srcPaletteFormat(other.srcPaletteFormat)
    {}

    void render(Buffer *const src, const Buffer *const srcPalette, const Colour &srcTransparent, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette, const Colour &destTransparent);

protected:
    struct UniformData {
        mat4 matrix alignas(64);
        Colour transparent alignas(16);
    };

    static QString uniformDataSrc(const QString &name, const GLint binding) {
        QString src;
        src +=
R"(
layout(std140, binding = $BINDING) uniform $NAMEBufferUniformData {
    mat4 matrix;
    Colour transparent;
} $NAMEBufferData;
)";
        stringMultiReplace(src, {
            {"$NAME", name},
            {"$BINDING", QString::number(binding)},
        });
        return src;
    }

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Buffer::Format srcFormat;
    const bool srcIndexed;
    const Buffer::Format srcPaletteFormat;
};

class SingleColourModelProgram : public RenderProgram {
public:
    SingleColourModelProgram(const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode)
    {
        updateKey(typeid(this), {});
    }
    SingleColourModelProgram(const SingleColourModelProgram &other) :
        RenderProgram(other)
    {}

    void render(Model *const model, const Colour &colour, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette);

protected:
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;
};

class VertexColourModelProgram : public RenderProgram {
public:
    VertexColourModelProgram(const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode)
    {
        updateKey(typeid(this), {});
    }
    VertexColourModelProgram(const VertexColourModelProgram &other) :
        RenderProgram(other)
    {}

    void render(Model *const model, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette);

protected:
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;
};

class BoundedPrimitiveProgram : public RenderProgram {
public:
    using RenderProgram::RenderProgram;

    void render(const std::array<Vec2, 2> &points, const Colour &colour, const Mat4 &toolSpaceTransform, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette);
};

class BoundedDistancePrimitiveProgram : public BoundedPrimitiveProgram {
public:
    BoundedDistancePrimitiveProgram(const bool filled, const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        BoundedPrimitiveProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        filled(filled)
    {
        updateKey(typeid(this), {filled});
    }
    BoundedDistancePrimitiveProgram(const BoundedDistancePrimitiveProgram &other) :
        BoundedPrimitiveProgram(other),
        filled(other.filled)
    {}

protected:
    const bool filled;

    virtual QString generateDistanceSource() const = 0;
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;
};

class RectProgram : public BoundedDistancePrimitiveProgram {
public:
    using BoundedDistancePrimitiveProgram::BoundedDistancePrimitiveProgram;

protected:
    virtual QString generateDistanceSource() const override;
};

class EllipseProgram : public BoundedDistancePrimitiveProgram {
public:
    using BoundedDistancePrimitiveProgram::BoundedDistancePrimitiveProgram;

protected:
    virtual QString generateDistanceSource() const override;
};

class ContourStencilProgram : public RenderProgram {
public:
    ContourStencilProgram(const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        storageBuffer(0), stencilTexture(0)
    {
        updateKey(typeid(this), {});

        glGenBuffers(1, &storageBuffer);
    }
    ContourStencilProgram(const ContourStencilProgram &other) :
        RenderProgram(other),
        storageBuffer(0), stencilTexture(0)
    {
        glGenBuffers(1, &storageBuffer);
    }

    virtual ~ContourStencilProgram() override {
        glDeleteBuffers(1, &storageBuffer);
    }
    void render(const std::vector<Stroke::Point> &points, const Colour &colour, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette);
    void postRender();

protected:
    GLuint storageBuffer;
    GLuint stencilTexture;

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;
};

class SmoothQuadProgram : public RenderProgram {
public:
    SmoothQuadProgram(const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        storageBuffer(0)
    {
        updateKey(typeid(this), {});

        glGenBuffers(1, &storageBuffer);
    }
    SmoothQuadProgram(const SmoothQuadProgram &other) :
        RenderProgram(other),
        storageBuffer(0)
    {
        glGenBuffers(1, &storageBuffer);
    }

    virtual ~SmoothQuadProgram() override {
        glDeleteBuffers(1, &storageBuffer);
    }

    void render(const std::vector<vec2> &points, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette);

protected:
    GLuint storageBuffer;

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;
};

class LineProgram : public RenderProgram {
public:
    struct alignas(16) Point {
        alignas(8) Vec2 pos;
        alignas(4) GLfloat width;
        alignas(4) GLfloat lineAbsPos;
        alignas(16) Colour colour;
    };

    LineProgram(const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        storageBuffer(0)
    {
        updateKey(typeid(this), {});

        glGenBuffers(1, &storageBuffer);
    }
    LineProgram(const LineProgram &other) :
        RenderProgram(other),
        storageBuffer(0)
    {
        glGenBuffers(1, &storageBuffer);
    }

    virtual ~LineProgram() override {
        glDeleteBuffers(1, &storageBuffer);
    }

    void render(const std::vector<Point> &points, const Colour &colour, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette);

protected:
    GLuint storageBuffer;

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;
};

class PatternProgram : public Program {
public:
    PatternProgram(const Pattern pattern, const Buffer::Format destFormat, const int blendMode) :
        Program(),
        pattern(pattern), destFormat(destFormat), blendMode(blendMode)
    {
        updateKey(typeid(this), {static_cast<int>(pattern), static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, blendMode});
    }
    PatternProgram(const PatternProgram &other) :
        Program(other),
        pattern(other.pattern), destFormat(other.destFormat), blendMode(other.blendMode)
    {
    }

    void render(const Mat4 &transform);

protected:
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Pattern pattern;
    const Buffer::Format destFormat;
    const int blendMode;
};

class PixelLineProgram : public RenderProgram {
public:
    PixelLineProgram(const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        storageBuffer(0)
    {
        updateKey(typeid(this), {});

        glGenBuffers(1, &storageBuffer);
    }
    PixelLineProgram(const PixelLineProgram &other) :
        RenderProgram(other),
        storageBuffer(0)
    {
        glGenBuffers(1, &storageBuffer);
    }

    virtual ~PixelLineProgram() override {
        glDeleteBuffers(1, &storageBuffer);
    }

    void render(const std::vector<Stroke::Point> &points, const Colour &colour, const Mat4 &worldToBuffer, const Mat4 &bufferToClip, Buffer *const dest, const Buffer *const destPalette);

protected:
    GLuint storageBuffer;

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;
};

class BrushDabProgram : public RenderProgram {
public:
    BrushDabProgram(const Brush::Dab::Type type, const int metric, const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        type(type), metric(metric),
        uniformData{},
        storageBuffer(0)
    {
        updateKey(typeid(this), {static_cast<int>(type), metric});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &storageBuffer);
    }
    BrushDabProgram(const BrushDabProgram &other) :
        RenderProgram(other),
        type(other.type), metric(other.metric),
        uniformData{}
    {
        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);
        glGenBuffers(1, &storageBuffer);
    }
    virtual ~BrushDabProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
        glDeleteBuffers(1, &storageBuffer);
    }

    void render(const std::vector<Stroke::Point> &points, const Brush::Dab &dab, const Colour &colour, const Mat4 &worldToBuffer, const Mat4 &bufferToClip, Buffer *const dest, const Buffer *const destPalette);

protected:
    struct alignas(16) UniformData {
        alignas(16) mat4 matrix;
        alignas(16) Colour colour;
        alignas(4) GLfloat hardness;
        alignas(4) GLfloat alpha;
    };

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Brush::Dab::Type type;
    const int metric;

    UniformData uniformData;
    GLuint storageBuffer;
};

class ColourPlaneProgram : public Program {
public:
    ColourPlaneProgram(const ColourSpace colourSpace, const bool useXAxis, const bool useYAxis, const Buffer::Format destFormat, const int blendMode, const bool quantise, const Buffer::Format quantisePaletteFormat) :
        Program(),
        colourSpace(colourSpace), useXAxis(useXAxis), useYAxis(useYAxis),
        destFormat(destFormat),
        blendMode(blendMode),
        quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
        uniformBuffer(0), uniformData()
    {
        updateKey(typeid(this), {static_cast<int>(colourSpace), useXAxis, useYAxis, static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, blendMode, quantise, static_cast<int>(quantisePaletteFormat.componentType), quantisePaletteFormat.componentSize, quantisePaletteFormat.componentCount});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    }
    ColourPlaneProgram(const ColourPlaneProgram &other) :
        Program(other),
        colourSpace(other.colourSpace), useXAxis(other.useXAxis), useYAxis(other.useYAxis),
        destFormat(other.destFormat),
        blendMode(other.blendMode),
        quantise(other.quantise), quantisePaletteFormat(other.quantisePaletteFormat),
        uniformBuffer(0), uniformData()
    {
        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);
    }
    virtual ~ColourPlaneProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    void render(const Colour &colour, const int xComponent, const int yComponent, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const quantisePalette);

protected:
    struct alignas(16) UniformData {
        alignas(16) Colour colour;
        alignas(8) ivec2 components;
    };

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const ColourSpace colourSpace;
    const bool useXAxis, useYAxis;
    const Buffer::Format destFormat;
    const int blendMode;
    const bool quantise;
    const Buffer::Format quantisePaletteFormat;

    GLuint uniformBuffer;
    UniformData uniformData;
};

class ColourPaletteProgram : public Program {
public:
    ColourPaletteProgram(const Buffer::Format destFormat, const int blendMode, const Buffer::Format paletteFormat) :
        Program(),
        destFormat(destFormat),
        blendMode(blendMode),
        paletteFormat(paletteFormat)
    {
        updateKey(typeid(this), {static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, blendMode, static_cast<int>(paletteFormat.componentType), paletteFormat.componentSize, paletteFormat.componentCount});
    }
    ColourPaletteProgram(const ColourPaletteProgram &other) :
        Program(other),
        destFormat(other.destFormat),
        blendMode(other.blendMode),
        paletteFormat(other.paletteFormat)
    {
    }

    void render(const Buffer *const palette, const QSize &cells, const Mat4 &worldToClip, Buffer *const dest);

protected:
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Buffer::Format destFormat;
    const int blendMode;
    const Buffer::Format paletteFormat;
};

class ColourPalettePickProgram : public ToolProgram {
public:
    ColourPalettePickProgram(const Buffer::Format format) :
        ToolProgram(),
        format(format),
        uniformBuffer(0)
    {
        updateKey(typeid(this), {static_cast<int>(format.componentType), format.componentSize, format.componentCount});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    }
    ColourPalettePickProgram(const ColourPalettePickProgram &other) :
        ToolProgram(other),
        format(other.format),
        uniformBuffer(0)
    {
        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    }
    virtual ~ColourPalettePickProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    Colour pick(const Buffer *const palette, const QSize &cells, const Vec2 &pos);

protected:
    struct alignas(16)StorageData {
        alignas(16) Colour colour;
    };

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Buffer::Format format;

    GLuint uniformBuffer;
};

class ColourConversionProgram : public ToolProgram {
public:
    ColourConversionProgram(const ColourSpace from, const ColourSpace to) :
        ToolProgram(),
        from(from), to(to)
    {
        updateKey(typeid(this), {static_cast<int>(from), static_cast<int>(to)});
    }
    ColourConversionProgram(const ColourConversionProgram &other) :
        ToolProgram(other),
        from(other.from), to(other.to)
    {
    }

    Colour convert(const Colour &colour);

protected:
    struct alignas(16) StorageData {
        alignas(16) Colour colour;
    };

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const ColourSpace from;
    const ColourSpace to;
};

class ColourPickProgram : public ToolProgram {
public:
    ColourPickProgram(const Buffer::Format format, const bool indexed, const Buffer::Format paletteFormat) :
        ToolProgram(),
        format(format), indexed(indexed), paletteFormat(paletteFormat)
    {
        updateKey(typeid(this), {static_cast<int>(format.componentType), format.componentSize, format.componentCount, indexed, static_cast<int>(paletteFormat.componentType), paletteFormat.componentSize, paletteFormat.componentCount});
    }
    ColourPickProgram(const ColourPickProgram &other) :
        ToolProgram(other),
        format(other.format), indexed(other.indexed), paletteFormat(other.paletteFormat)
    {
    }

    Colour pick(const Buffer *const src, const Buffer *const srcPalette, const Vec2 &pos);

protected:
    struct alignas(16) StorageData {
        alignas(16) Colour colour;
    };

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Buffer::Format format;
    const bool indexed;
    const Buffer::Format paletteFormat;
};

class ProgramManager {
public:
    explicit ProgramManager() :
        programs()
    {
    }
    ~ProgramManager() {
        const auto keys = programs.keys();
        for (const auto &key : keys) {
            delete programs.value(key).first;
        }
    }

    bool contains(const Program::Key &key) const {
        return programs.contains(key);
    }

    QOpenGLShaderProgram *grab(const Program::Key &key, std::function<QOpenGLShaderProgram *()> createFunc = nullptr) {
        Q_ASSERT(contains(key) || createFunc);
        if (!contains(key)) {
            programs.insert(key, std::make_pair(createFunc(), 0));
        }
        programs[key].second++;
        return programs.value(key).first;
    }
    void release(const Program::Key &key) {
        Q_ASSERT(programs.contains(key));
        programs[key].second--;
        if (programs.value(key).second == 0) {
            delete programs.take(key).first;
        }
    }

protected:
    QMap<Program::Key, std::pair<QOpenGLShaderProgram *, int>> programs;
};

} // namespace GfxPaint

#endif // PROGRAM_H
