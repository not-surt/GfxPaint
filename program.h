#ifndef PROGRAM_H
#define PROGRAM_H

#include "opengl.h"

#include <typeindex>
#include <QOpenGLShaderProgram>
#include "types.h"
#include "buffer.h"
#include "brush.h"

namespace GfxPaint {

class ProgramComponent : protected OpenGL {
public:
    ProgramComponent() :
        OpenGL(true),
        srcs(),
        uniformBuffer(0), storageBuffer(0)
    {
        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glGenBuffers(1, &storageBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, storageBuffer);
    }
    virtual ~ProgramComponent() {
        glDeleteBuffers(1, &storageBuffer);
        glDeleteBuffers(1, &uniformBuffer);
    }

    QMap<QOpenGLShader::ShaderType, QString> srcs;

    GLuint uniformBuffer;
    GLuint storageBuffer;
};

class TransformProgramComponent : public ProgramComponent {
public:
    TransformProgramComponent() :
        transform()
    {
        srcs[QOpenGLShader::Vertex] = "";
        srcs[QOpenGLShader::Fragment] = "";
    }

    QTransform transform;
};

typedef QList<ProgramComponent *> ProgramComponentList;

class Program : protected OpenGL {
public:
    typedef std::pair<std::type_index, QList<int>> Key;

    Program(const std::type_index type, const QList<int> &values);
    virtual ~Program();

    QOpenGLShaderProgram &program();

protected:
    virtual QOpenGLShaderProgram *createProgram() const = 0;

    const Key key;

    QList<ProgramComponent> components;

private:
    QOpenGLShaderProgram *m_program;
};

class WidgetProgram : public Program {
public:
    WidgetProgram(const Buffer::Format srcFormat, const bool srcIndexed, const Buffer::Format srcPaletteFormat) :
        Program(typeid(WidgetProgram), {static_cast<int>(srcFormat.componentType), srcFormat.componentSize, srcFormat.componentCount, static_cast<int>(srcIndexed)}),
        srcFormat(srcFormat), srcIndexed(srcIndexed), srcPaletteFormat(srcPaletteFormat)
    {}

    void render(Buffer *const src);

protected:
    virtual QOpenGLShaderProgram *createProgram() const override;

    const Buffer::Format srcFormat;
    const bool srcIndexed;
    const Buffer::Format srcPaletteFormat;
};

class BufferProgram : public Program {
public:
    BufferProgram(const Buffer::Format srcFormat, const bool srcIndexed, const Buffer::Format srcPaletteFormat, const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const Blender blender) :
        Program(typeid(BufferProgram), {static_cast<int>(srcFormat.componentType), srcFormat.componentSize, srcFormat.componentCount, static_cast<int>(srcIndexed), static_cast<int>(srcPaletteFormat.componentType), srcPaletteFormat.componentSize, srcPaletteFormat.componentCount, static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, static_cast<int>(destIndexed), static_cast<int>(destPaletteFormat.componentType), destPaletteFormat.componentSize, destPaletteFormat.componentCount, static_cast<int>(blender)}),
        srcFormat(srcFormat), srcIndexed(srcIndexed), srcPaletteFormat(srcPaletteFormat),
        destFormat(destFormat), destIndexed(destIndexed), destPaletteFormat(destPaletteFormat),
        blender(blender),
        uniformBuffer(0), uniformData()
    {
        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);
    }
    ~BufferProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }


    void render(Buffer *const src, const Buffer *const srcPalette, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette);

protected:
    struct UniformData {
        GLfloat matrix[3][3];
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const Buffer::Format srcFormat;
    const bool srcIndexed;
    const Buffer::Format srcPaletteFormat;
    const Buffer::Format destFormat;
    const bool destIndexed;
    const Buffer::Format destPaletteFormat;
    const Blender blender;

    GLuint uniformBuffer;
    UniformData uniformData;
};

class DabProgram : public Program {
public:
    DabProgram(const Dab::Type type, const Metric metric, const Buffer::Format destFormat, const bool destIndexed, const Buffer::Format destPaletteFormat, const Blender blender) :
        Program(typeid(DabProgram), {static_cast<int>(type), static_cast<int>(metric), static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, static_cast<int>(destIndexed), static_cast<int>(destPaletteFormat.componentType), destPaletteFormat.componentSize, destPaletteFormat.componentCount, static_cast<int>(blender)}),
        type(type), metric(metric),
        destFormat(destFormat), destIndexed(destIndexed), destPaletteFormat(destPaletteFormat),
        blender(blender),
        uniformBuffer(0), uniformData()
    {
        glGenBuffers(1, &uniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);
    }
    ~DabProgram() override {
        glDeleteBuffers(1, &uniformBuffer);
    }

    void render(const Dab &dab, const QColor &colour, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette);

protected:
    struct UniformData {
        GLfloat matrix[3][3];
        GLfloat hardness;
        GLfloat alpha;
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const Dab::Type type;
    const Metric metric;
    const Buffer::Format destFormat;
    const bool destIndexed;
    const Buffer::Format destPaletteFormat;
    const Blender blender;

    GLuint uniformBuffer;
    UniformData uniformData;
};

class ColourSliderProgram : public Program {
public:
    ColourSliderProgram(const ColourSpace colourSpace, const int component, const Buffer::Format destFormat, const Blender blender) :
        Program(typeid(ColourSliderProgram), {static_cast<int>(colourSpace), component, static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, static_cast<int>(blender)}),
        colourSpace(colourSpace), component(component),
        destFormat(destFormat),
        blender(blender)
    {}

    void render(const QColor &colour, const ColourSpace colourSpace, const int component, const QTransform &transform, Buffer *const dest);

protected:
    struct UniformData {
        GLfloat colour[4];
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const ColourSpace colourSpace;
    const int component;
    const Buffer::Format destFormat;
    const Blender blender;

    UniformData uniformData;
};

class PatternProgram : public Program {
public:
    PatternProgram(const Pattern pattern, const Buffer::Format destFormat, const Blender blender) :
        Program(typeid(PatternProgram), {static_cast<int>(pattern), static_cast<int>(destFormat.componentType), destFormat.componentSize, destFormat.componentCount, static_cast<int>(blender)}),
        pattern(pattern), destFormat(destFormat), blender(blender)
    {}

    void render(const QTransform &transform);

protected:
    virtual QOpenGLShaderProgram *createProgram() const override;

    const Pattern pattern;
    const Buffer::Format destFormat;
    const Blender blender;
};

class ColourConversionProgram : public Program {
public:
    ColourConversionProgram(const ColourSpace from, const ColourSpace to) :
        Program(typeid(ColourConversionProgram), {static_cast<int>(from), static_cast<int>(to)}),
        from(from), to(to),
        storageBuffer(0), storageData{{0.0}}
    {
        glGenBuffers(1, &storageBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    }
    virtual ~ColourConversionProgram() override {
        glDeleteBuffers(1, &storageBuffer);
    }

    void convert(const float from[4], float to[4]);
    void convert(float value[4]) {
        convert(value, value);
    }

protected:
    struct StorageData {
        float colour[4];
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const ColourSpace from;
    const ColourSpace to;

    GLuint storageBuffer;
    StorageData storageData;
};

class ColourPickProgram : public Program {
public:
    ColourPickProgram(const Buffer::Format format) :
        Program(typeid(ColourPickProgram), {static_cast<int>(format.componentType), format.componentSize, format.componentCount}),
        format(format),
        storageBuffer(0), storageData{{0.0}}
    {
        glGenBuffers(1, &storageBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    }
    ~ColourPickProgram() override {
        glDeleteBuffers(1, &storageBuffer);
    }

    QColor pick(Buffer *const src, const QPointF pos);

protected:
    struct StorageData {
        float colour[4];
    };

    virtual QOpenGLShaderProgram *createProgram() const override;

    const Buffer::Format format;

    GLuint storageBuffer;
    StorageData storageData;
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
