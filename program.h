#ifndef PROGRAM_H
#define PROGRAM_H

#include "opengl.h"

#include <typeindex>
#include <QOpenGLShaderProgram>
#include "types.h"
#include "buffer.h"
#include "brush.h"
#include "model.h"
#include "utils.h"

namespace GfxPaint {

static const std::list<QOpenGLShader::ShaderTypeBit> programStages = {
    QOpenGLShader::Geometry, QOpenGLShader::Vertex, QOpenGLShader::Fragment, QOpenGLShader::Compute,
};

struct SimpleProgramState {
    GLint attributeLocation = 0;
    GLint uniformLocation = 0;

    GLint nextAttributeLocation() {
        return attributeLocation++;
    }
    GLint nextUniformLocation() {
        return uniformLocation++;
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
        for (auto stage : programStages) {
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

        for (const auto &stage : programStages) {
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
    typedef std::pair<std::type_index, QList<int>> Key;

    Program();
    virtual ~Program();

    QOpenGLShaderProgram &program();

protected:
    void updateKey(const std::type_index type, const QList<int> &values) {
        key.first = type;
        key.second.append(values);
    }
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const = 0;
    QOpenGLShaderProgram *createProgram() const;

private:
    Key key;
    QOpenGLShaderProgram *m_program;
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

    void render(Buffer *const src, const QMatrix4x4 &transform);

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

    void render(Buffer *const src, const Buffer *const srcPalette, const Colour &srcTransparent, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette, const Colour &destTransparent);

protected:
    struct UniformData {
        mat4 matrix alignas(16);
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

class ModelProgram : public RenderProgram {
public:
    ModelProgram(const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        uniformData{}
    {
        updateKey(typeid(this), {});
    }

    void render(Model *const model, const Colour &colour, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette);

protected:
    struct UniformData {
        mat4 matrix alignas(16);
    };

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    UniformData uniformData;
};

class PatternProgram : public Program {
public:
    PatternProgram(const Pattern pattern, const Buffer::Format destFormat, const int blendMode) :
        Program(),
        pattern(pattern), destFormat(destFormat), blendMode(blendMode)
    {
        updateKey(typeid(this), {static_cast<int>(pattern), static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, blendMode});
    }

    void render(const QMatrix4x4 &transform);

protected:
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Pattern pattern;
    const Buffer::Format destFormat;
    const int blendMode;
};

class DabProgram : public RenderProgram {
public:
    DabProgram(const Brush::Dab::Type type, const int metric, const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
        RenderProgram(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode),
        type(type), metric(metric),
        uniformData{}
    {
        updateKey(typeid(this), {static_cast<int>(type), metric});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);
    }
    virtual ~DabProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    void render(const Brush::Dab &dab, const Colour &colour, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette);

protected:
    struct UniformData {
        mat4 matrix alignas(16);
        Colour colour alignas(16);
        GLfloat hardness alignas(16);
        GLfloat alpha alignas(16);
    };

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Brush::Dab::Type type;
    const int metric;

    UniformData uniformData;
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
    virtual ~ColourPlaneProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    void render(const Colour &colour, const int xComponent, const int yComponent, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const quantisePalette);

protected:
    struct UniformData {
        Colour colour alignas(16);
        ivec2 components alignas(16);
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

    void render(const Buffer *const palette, const QSize size, const QSize swatchSize, const QSize cells, const QMatrix4x4 &transform, Buffer *const dest);

protected:
    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const Buffer::Format destFormat;
    const int blendMode;
    const Buffer::Format paletteFormat;
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
    virtual ~ToolProgram() override {
        glDeleteBuffers(1, &storageBuffer);
    }

protected:
    GLuint storageBuffer;
};

class ColourPlanePickProgram : public ToolProgram {
public:
    ColourPlanePickProgram(const ColourSpace colourSpace, const int xComponent, const int yComponent, const bool quantise, const Buffer::Format quantisePaletteFormat) :
        ToolProgram(),
        colourSpace(colourSpace), xComponent(xComponent), yComponent(yComponent),
        quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
        uniformBuffer(0), uniformData()
    {
        updateKey(typeid(this), {static_cast<int>(colourSpace), xComponent, yComponent, quantise, static_cast<int>(quantisePaletteFormat.componentType), quantisePaletteFormat.componentSize, quantisePaletteFormat.componentCount});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    }
    virtual ~ColourPlanePickProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    Colour pick(const Colour &colour, const QVector2D &pos, const Buffer *const quantisePalette);

protected:
    struct UniformData {
        Colour colour alignas(16);
    };

    virtual QString generateSource(QOpenGLShader::ShaderTypeBit stage) const override;

    const ColourSpace colourSpace;
    const int xComponent, yComponent;
    const bool quantise;
    const Buffer::Format quantisePaletteFormat;

    GLuint uniformBuffer;
    UniformData uniformData;
};

class ColourPalettePickProgram : public ToolProgram {
public:
    ColourPalettePickProgram(const Buffer::Format format) :
        ToolProgram(),
        format(format)
    {
        updateKey(typeid(this), {static_cast<int>(format.componentType), format.componentSize, format.componentCount});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    }
    virtual ~ColourPalettePickProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    Colour pick(const Buffer *const src, const QVector2D &pos, const QMatrix4x4 &transform);

protected:
    struct StorageData {
        Colour colour alignas(16);
    };
    struct UniformData {
        mat4 matrix alignas(16);
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

    Colour convert(const Colour &colour);

protected:
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

    Colour pick(const Buffer *const dest, const Buffer *const destPalette, const QVector2D &pos);

protected:
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
        for (const auto &key : programs.keys()) {
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
