#include "program.h"

#include <functional>
#include "application.h"
#include "rendermanager.h"
#include "utils.h"

namespace GfxPaint {

Program::Program(const std::type_index type, const QList<int> &values) :
    OpenGL(true),
    key(type, values),
    m_program(nullptr)
{
}

Program::~Program() {
    if (m_program) {
        qApp->renderManager.programManager.release(key);
    }
}

QOpenGLShaderProgram &Program::program() {
    if (!m_program) {
        m_program = qApp->renderManager.programManager.grab(key, [this](){return createProgram();});
    }
    return *m_program;
}

QOpenGLShaderProgram *WidgetProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::modelShaderPart(Model::UnitQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::bufferShaderPart("src", 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
    fragSrc += RenderManager::widgetOutputShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void WidgetProgram::render(Buffer *const src)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    program.setUniformValue("matrix", qApp->renderManager.unitToClipTransform * qApp->renderManager.flipTransform);

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), src->width(), src->height());

    glUniform1i(program.uniformLocation("srcTexture"), 0);
    src->bindTextureUnit(0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QOpenGLShaderProgram *BufferProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::modelShaderPart(Model::UnitQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::bufferShaderPart("src", 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
    fragSrc += RenderManager::bufferShaderPart("dest", 2, destFormat, destIndexed, 3, destPaletteFormat);
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void BufferProgram::render(Buffer *const src, const Buffer *const srcPalette, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    program.setUniformValue("matrix", transform);

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), src->width(), src->height());

    glUniform1i(program.uniformLocation("srcTexture"), 0);
    src->bindTextureUnit(0);

    if (srcIndexed && srcPalette) {
        glUniform1i(program.uniformLocation("srcPalette"), 1);
        srcPalette->bindTextureUnit(1);
    }

    if (dest) {
        dest->bindTextureUnit(2);
        glUniform1i(program.uniformLocation("destTexture"), 2);

        if (destIndexed && destPalette) {
            glUniform1i(program.uniformLocation("destPalette"), 3);
            destPalette->bindTextureUnit(3);
        }
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QOpenGLShaderProgram *DabProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::modelShaderPart(type == Dab::Type::Pixel ? Model::SingleVertex : Model::ClipQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::dabShaderPart("src", type, metric);
    fragSrc += RenderManager::bufferShaderPart("dest", 0, destFormat, destIndexed, 1, destPaletteFormat);
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, destPaletteFormat, blendMode, composeMode);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void DabProgram::render(const Dab &dab, const QColor &colour, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);

    program.setUniformValue("matrix", dab.transform() * transform);

    glUniform4f(program.uniformLocation("srcColour"), static_cast<GLfloat>(colour.redF()), static_cast<GLfloat>(colour.greenF()), static_cast<GLfloat>(colour.blueF()), static_cast<GLfloat>(colour.alphaF()));
    glUniform1f(program.uniformLocation("srcHardness"), static_cast<GLfloat>(dab.hardness));
    glUniform1f(program.uniformLocation("srcOpacity"), static_cast<GLfloat>(dab.opacity));

    dest->bindTextureUnit(0);
    glUniform1i(program.uniformLocation("destTexture"), 0);
    //dest->bindImageUnit(0);
    //glUniform1i(program.uniformLocation("destImage"), 0);
    if (destIndexed && destPalette) {
        glUniform1i(program.uniformLocation("destPalette"), 1);
        destPalette->bindTextureUnit(1);
    }

    if (dab.type == Dab::Type::Pixel) {
        glDrawArrays(GL_POINTS, 4, 1);
    }
    else {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    //glTextureBarrier();
}

QOpenGLShaderProgram *ColourSliderProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::modelShaderPart(Model::UnitQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::colourSliderShaderPart("src", colourSpace, component);
    fragSrc += RenderManager::bufferShaderPart("dest", 0, destFormat, false, 0, Buffer::Format());
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, false, Buffer::Format(), blendMode, 3);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void ColourSliderProgram::render(const QColor &colour, const ColourSpace colourSpace, const int component, const QTransform &transform, Buffer *const dest)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    GLfloat col[4]{(GLfloat)colour.redF(), (GLfloat)colour.greenF(), (GLfloat)colour.blueF(), (GLfloat)colour.alphaF()};
    memcpy(uniformData.colour, col, sizeof(GLfloat) * 4);
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);

    program.setUniformValue("matrix", transform);

    dest->bindTextureUnit(0);
    glUniform1i(program.uniformLocation("destTexture"), 0);
    dest->bindImageUnit(0);
    glUniform1i(program.uniformLocation("destImage"), 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QOpenGLShaderProgram *ColourSliderMarkerProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

//    QString vertSrc;
//    vertSrc += RenderManager::headerShaderPart();
//    vertSrc += RenderManager::modelShaderPart(Model::UnitQuad);
//    vertSrc += RenderManager::vertexMainShaderPart();
//    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

//    QString fragSrc;
//    fragSrc += RenderManager::headerShaderPart();
//    fragSrc += RenderManager::colourSliderShaderPart("src", colourSpace, component);
//    fragSrc += RenderManager::bufferShaderPart("dest", 0, destFormat, false, 0, Buffer::Format());
//    fragSrc += RenderManager::blenderShaderPart(blender);
//    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, false, Buffer::Format());
//    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

//    program->link();
    return program;
}

void ColourSliderMarkerProgram::render(const QTransform &transform, const QColor &colour0, const QColor &colour1)
{

}

QOpenGLShaderProgram *ColourSliderPickProgram::createProgram() const
{
    QOpenGLShaderProgram *const program = new QOpenGLShaderProgram();

    QString compSrc;
    compSrc += RenderManager::headerShaderPart();
    compSrc += RenderManager::colourSliderShaderPart("src", colourSpace, component);
    compSrc +=
R"(
uniform float pos;
layout(std430, binding = 0) buffer storageData
{
    vec4 colour;
};
layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = src(vec2(pos, 0.5));
}
)";
    program->addShaderFromSourceCode(QOpenGLShader::Compute, compSrc);

    program->link();
    return program;
}

QColor ColourSliderPickProgram::pick(QColor colour, const float pos)
{
    typedef qreal (QColor::*const Getter)() const;
    static const Getter getters[4] = {&QColor::redF, &QColor::greenF, &QColor::blueF, &QColor::alphaF};
    for (int i = 0; i < 4; ++i) storageData.colour[i] = static_cast<GLfloat>((colour.*getters[i])());
    program().bind();

    glUniform1f(program().uniformLocation("pos"), static_cast<GLfloat>(pos));

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(storageData), &storageData, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(StorageData), &storageData, GL_STREAM_READ);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(StorageData), &storageData);

    typedef void (QColor::*const Setter)(qreal value);
    static const Setter setters[4] = {&QColor::setRedF, &QColor::setGreenF, &QColor::setBlueF, &QColor::setAlphaF};
    for (int i = 0; i < 4; ++i) (colour.*setters[i])(static_cast<qreal>(storageData.colour[i]));
    return colour;
}

QOpenGLShaderProgram *PatternProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/background.vert");

    //program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/background.frag");
    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::patternShaderPart("src", Pattern::Checkers);
    fragSrc += RenderManager::bufferShaderPart("dest", 0, destFormat, false, 0, Buffer::Format());
    //fragSrc += RenderManager::fragmentMainShaderPart(destFormat);
    fragSrc += RenderManager::widgetOutputShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void PatternProgram::render(const QTransform &transform)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();
    program.setUniformValue("matrix", (transform).inverted());

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

QOpenGLShaderProgram *ColourConversionProgram::createProgram() const
{
    QOpenGLShaderProgram *const program = new QOpenGLShaderProgram();

    QString compSrc;
    compSrc += RenderManager::headerShaderPart();
    compSrc += fileToString(":/shaders/thirdparty/ColorSpaces.inc.glsl");
    compSrc +=
R"(
layout(std430, binding = 0) buffer storageData
{
    vec4 colour;
};
layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = vec4(%1_to_%2(colour.rgb), colour.a);
}
)";
    compSrc = compSrc.arg(colourSpaceInfo[from].funcName).arg(colourSpaceInfo[to].funcName);
    program->addShaderFromSourceCode(QOpenGLShader::Compute, compSrc);

    program->link();
    return program;
}

void ColourConversionProgram::convert(const float from[], float to[]) {
    memcpy(&storageData, from, sizeof(storageData));
    program().bind();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(StorageData), &storageData, GL_STREAM_READ);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(StorageData), &storageData);
    memcpy(to, &storageData, sizeof(StorageData));
}

QOpenGLShaderProgram *ColourPickProgram::createProgram() const
{
    QOpenGLShaderProgram *const program = new QOpenGLShaderProgram();

    QString compSrc;
    compSrc += RenderManager::headerShaderPart();
    compSrc += RenderManager::bufferShaderPart("src", 0, format, false, 0, Buffer::Format());
    compSrc +=
R"(
uniform vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    vec4 colour;
};
layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = vec4(src(pos));
}
)";
    program->addShaderFromSourceCode(QOpenGLShader::Compute, compSrc);

    program->link();
    return program;
}

QColor ColourPickProgram::pick(Buffer *const src, const QPointF pos)
{
    QColor colour;
    colour.setRgbF(0.0, 0.0, 0.0, 1.0);
    program().bind();
    glUniform1i(program().uniformLocation("srcTexture"), 0);
    src->bindTextureUnit(0);
    glUniform2f(program().uniformLocation("pos"), static_cast<GLfloat>(pos.x()), static_cast<GLfloat>(pos.y()));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(StorageData), nullptr, GL_STREAM_READ);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(StorageData), &storageData);
    typedef void (QColor::*const Setter)(qreal value);
    static const Setter setters[4] = {&QColor::setRedF, &QColor::setGreenF, &QColor::setBlueF, &QColor::setAlphaF};
    for (int i = 0; i < src->format().componentSize; ++i) (colour.*setters[i])(static_cast<qreal>(storageData.colour[i]));
    return colour;
}

} // namespace GfxPaint
