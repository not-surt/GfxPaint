#ifndef PROGRAM_H
#define PROGRAM_H

#include "opengl.h"

#include <typeindex>
#include <QOpenGLShaderProgram>
#include "type.h"
#include "buffer.h"
#include "brush.h"
#include "model.h"
#include "utils.h"

namespace GfxPaint {

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
    virtual QOpenGLShader::ShaderType stages() const = 0;
    virtual QString definitions(SimpleProgramState &state) const { return QString(); }
    virtual QString main(SimpleProgramState &state) const { return QString(); }
    virtual void bind() const {}
};

struct VertexPositionProgramComponent {
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
        static const QList<QOpenGLShader::ShaderTypeBit> stages = {
            QOpenGLShader::Geometry, QOpenGLShader::Vertex, QOpenGLShader::Fragment, QOpenGLShader::Compute,
        };

        QOpenGLShaderProgram *program = new QOpenGLShaderProgram();
        QMap<QOpenGLShader::ShaderTypeBit, QString> definitions, main;
        SimpleProgramState state;
        for (auto stage : stages) {
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
        static const QList<QOpenGLShader::ShaderTypeBit> stages = {
            QOpenGLShader::Geometry, QOpenGLShader::Vertex, QOpenGLShader::Fragment, QOpenGLShader::Compute,
        };

        for (auto stage : stages) {

        }
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
    virtual QOpenGLShaderProgram *createProgram() const = 0;

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

    void render(Buffer *const src);

protected:
    virtual QOpenGLShaderProgram *createProgram() const override;

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

    void render(Buffer *const src, const Buffer *const srcPalette, const Colour &transparent, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette);

protected:
    struct BufferUniformData {
        mat4 matrix;
        Colour transparent;
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

    virtual QOpenGLShaderProgram *createProgram() const override;

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

    void render(Model *const model, const Colour &colour, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette);

protected:
    struct UniformData {
        mat3 matrix;
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    UniformData uniformData;
};

class DabProgram : public RenderProgram {
public:
    DabProgram(const Dab::Type type, const int metric, const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const int blendMode, const int composeMode) :
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

    void render(const Dab &dab, const Colour &colour, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette);

protected:
    struct UniformData {
        mat3 matrix;
        Colour colour;
        GLfloat hardness;
        GLfloat alpha;
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const Dab::Type type;
    const int metric;

    UniformData uniformData;
};

class ColourSliderProgram : public Program {
public:
    ColourSliderProgram(const ColourSpace colourSpace, const int component, const Buffer::Format destFormat, const int blendMode, const bool quantise, const Buffer::Format quantisePaletteFormat) :
        Program(),
        colourSpace(colourSpace), component(component),
        destFormat(destFormat),
        blendMode(blendMode),
        quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
        uniformBuffer(0), uniformData()
    {
        updateKey(typeid(this), {static_cast<int>(colourSpace), component, static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, blendMode, quantise, static_cast<int>(quantisePaletteFormat.componentType), quantisePaletteFormat.componentSize, quantisePaletteFormat.componentCount});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    }
    virtual ~ColourSliderProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    void render(const Colour &colour, const ColourSpace colourSpace, const int component, const QTransform &transform, Buffer *const dest, const Buffer *const quantisePalette);

protected:
    struct UniformData {
        Colour colour;
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const ColourSpace colourSpace;
    const int component;
    const Buffer::Format destFormat;
    const int blendMode;
    const bool quantise;
    const Buffer::Format quantisePaletteFormat;

    GLuint uniformBuffer;
    UniformData uniformData;
};

class ColourPlaneProgram : public Program {
public:
    ColourPlaneProgram(const ColourSpace colourSpace, const int componentX, const int componentY, const Buffer::Format destFormat, const int blendMode, const bool quantise) :
        Program(),
        colourSpace(colourSpace), componentX(componentX), componentY(componentY),
        destFormat(destFormat),
        blendMode(blendMode),
        quantise(quantise),
        uniformBuffer(0), uniformData()
    {
        updateKey(typeid(this), {static_cast<int>(colourSpace), componentX, componentY, static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, blendMode, quantise});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    }
    virtual ~ColourPlaneProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    void render(const Colour &colour, const ColourSpace colourSpace, const int componentX, const int componentY, const QTransform &transform, Buffer *const dest);

protected:
    struct UniformData {
        Colour colour;
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const ColourSpace colourSpace;
    const int componentX, componentY;
    const Buffer::Format destFormat;
    const int blendMode;
    const bool quantise;

    GLuint uniformBuffer;
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

    void render(const QTransform &transform);

protected:
    virtual QOpenGLShaderProgram *createProgram() const override;

    const Pattern pattern;
    const Buffer::Format destFormat;
    const int blendMode;
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

class ColourSliderPickProgram : public ToolProgram {
public:
    ColourSliderPickProgram(const ColourSpace colourSpace, const int component, const bool quantise, const Buffer::Format quantisePaletteFormat) :
        ToolProgram(),
        colourSpace(colourSpace), component(component),
        quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
        uniformBuffer(0), uniformData()
    {
        updateKey(typeid(this), {static_cast<int>(colourSpace), component, quantise, static_cast<int>(quantisePaletteFormat.componentType), quantisePaletteFormat.componentSize, quantisePaletteFormat.componentCount});

        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    }
    virtual ~ColourSliderPickProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    Colour pick(const Colour &colour, const float pos, const Buffer *const quantisePalette);

protected:
    struct UniformData {
        Colour colour;
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const ColourSpace colourSpace;
    const int component;
    const bool quantise;
    const Buffer::Format quantisePaletteFormat;

    GLuint uniformBuffer;
    UniformData uniformData;
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
    virtual QOpenGLShaderProgram *createProgram() const override;

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

    Colour pick(const Buffer *const src, const Buffer *const srcPalette, const QPointF pos);

protected:
    virtual QOpenGLShaderProgram *createProgram() const override;

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
        for (auto key : programs.keys()) {
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
