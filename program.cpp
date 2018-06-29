#include "program.h"

#include <functional>
#include "application.h"
#include "rendermanager.h"
#include "utils.h"

namespace GfxPaint {

Program::Program() :
    OpenGL(true),
    key(typeid(Program), {}),
    m_program(nullptr)
{
}

Program::~Program() {
    if (m_program) qApp->renderManager.programManager.release(key);
}

QOpenGLShaderProgram &Program::program() {
    if (!m_program) m_program = qApp->renderManager.programManager.grab(key, [this](){return createProgram();});
    return *m_program;
}

QOpenGLShaderProgram *RenderedWidgetProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::bufferShaderPart("src", 0, 0, 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
    fragSrc += RenderManager::widgetOutputShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void RenderedWidgetProgram::render(Buffer *const src)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

//    program.setUniformValue("matrix", qApp->renderManager.unitToClipTransform * qApp->renderManager.flipTransform);
    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, (qApp->renderManager.flipTransform * qApp->renderManager.unitToClipTransform).data());

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), src->width(), src->height());

    qApp->renderManager.bindBufferShaderPart(program, "src", 0, 0, src);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QOpenGLShaderProgram *BufferProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::bufferShaderPart("src", 0, 0, 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
    fragSrc += RenderManager::bufferShaderPart("dest", 2, 2, 2, destFormat, destIndexed, 3, destPaletteFormat);
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 3, destPaletteFormat, blendMode, composeMode);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void BufferProgram::render(Buffer *const src, const Buffer *const srcPalette, const Colour &transparent, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.data());

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), src->width(), src->height());

    BufferUniformData uniformData;
    memcpy(&uniformData.matrix, transform.data(), sizeof(uniformData.matrix));
    uniformData.transparent = transparent;

    const GLuint blockIndex = glGetUniformBlockIndex(program.programId(), "srcUniformData");
    const GLuint bindingPoint = 2;
    glUniformBlockBinding(program.programId(), blockIndex, bindingPoint);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "src", 0, 0, src, srcIndexed, 1, 1, srcPalette);

    if (dest) {
        qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 2, 2, dest, destIndexed, 3, 3, destPalette);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QOpenGLShaderProgram *ModelProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::modelVertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::modelFragmentShaderPart("src");
    fragSrc += RenderManager::bufferShaderPart("dest", 0, 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void ModelProgram::render(Model *const model, const Colour &colour, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette) {
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.data());

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, 0, dest, destIndexed, 1, 1, destPalette);

    model->render();
}

QOpenGLShaderProgram *DabProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::attributelessShaderPart(type == Brush::Dab::Type::Pixel ? AttributelessModel::SingleVertex : AttributelessModel::ClipQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::dabShaderPart("src", type, metric);
    fragSrc += RenderManager::bufferShaderPart("dest", 0, 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void DabProgram::render(const Brush::Dab &dab, const Colour &colour, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    QMatrix4x4 matrix = transform * dab.transform();

    memcpy(&uniformData.matrix, matrix.data(), sizeof(uniformData.matrix));
    uniformData.colour = colour;
    uniformData.hardness = static_cast<GLfloat>(dab.hardness);
    uniformData.alpha = static_cast<GLfloat>(dab.opacity);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, matrix.data());

    glUniform4fv(program.uniformLocation("srcColour"), 1, colour.rgba.data());
    glUniform1f(program.uniformLocation("srcHardness"), static_cast<GLfloat>(dab.hardness));
    glUniform1f(program.uniformLocation("srcOpacity"), static_cast<GLfloat>(dab.opacity));

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, 0, dest, destIndexed, 1, 1, destPalette);

    if (dab.type == Brush::Dab::Type::Pixel) {
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
    vertSrc += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::colourSliderShaderPart("src", colourSpace, component, quantise, 1, quantisePaletteFormat);
    fragSrc += RenderManager::bufferShaderPart("dest", 0, 0, 0, destFormat, false, 0, Buffer::Format());
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, false, 0, Buffer::Format(), blendMode, RenderManager::composeModeDefault);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void ColourSliderProgram::render(const Colour &colour, const ColourSpace colourSpace, const int component, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const quantisePalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    UniformData uniformData = {colour};
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.data());

    qApp->renderManager.bindBufferShaderPart(program, "dest", 0, 0, dest);
    if (quantise && quantisePalette) {
        qApp->renderManager.bindBufferShaderPart(program, "quantisePalette", 1, 1, quantisePalette);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QOpenGLShaderProgram *ColourPlaneProgram::createProgram() const
{
    return nullptr;
}

void ColourPlaneProgram::render(const Colour &colour, const ColourSpace colourSpace, const int componentX, const int componentY, const QMatrix4x4 &transform, Buffer *const dest)
{
}

QOpenGLShaderProgram *PatternProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/background.vert");

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::patternShaderPart("src", Pattern::Checkers);
    fragSrc += RenderManager::bufferShaderPart("dest", 0, 0, 0, destFormat, false, 0, Buffer::Format());
    //fragSrc += RenderManager::fragmentMainShaderPart(destFormat);
    fragSrc += RenderManager::widgetOutputShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void PatternProgram::render(const QMatrix4x4 &transform)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();
    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.inverted().data());

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

QOpenGLShaderProgram *ColourSliderPickProgram::createProgram() const
{
    QOpenGLShaderProgram *const program = new QOpenGLShaderProgram();

    QString compSrc;
    compSrc += RenderManager::headerShaderPart();
    compSrc += RenderManager::colourSliderShaderPart("src", colourSpace, component, quantise, 0, quantisePaletteFormat);
    compSrc +=
R"(
uniform layout(location = 2) float pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
};

layout(local_size_x = 1, local_size_y = 1) in;
void main() {
    srcColour = colour;
    colour = src(vec2(pos, 0.0));
}
)";
    program->addShaderFromSourceCode(QOpenGLShader::Compute, compSrc);

    program->link();
    return program;
}

Colour ColourSliderPickProgram::pick(const Colour &colour, const float pos, const Buffer *const quantisePalette)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    UniformData uniformData = {colour};
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    glUniform1f(2, static_cast<GLfloat>(pos));
    if (quantise && quantisePalette) {
        qApp->renderManager.bindBufferShaderPart(program, "quantisePalette", 0, 0, quantisePalette);
    }

    Colour storageData = colour;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), &storageData, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);

    return storageData;
}

QOpenGLShaderProgram *ColourConversionProgram::createProgram() const
{
    QOpenGLShaderProgram *const program = new QOpenGLShaderProgram();

    QString compSrc;
    compSrc += RenderManager::headerShaderPart();
    compSrc += fileToString(":/shaders/ColorSpaces.inc.glsl");
    compSrc +=
R"(
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
};

layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = Colour(vec4(%1_to_%2(colour.rgba.rgb), colour.rgba.a), INDEX_INVALID);
}
)";
    stringMultiReplace(compSrc, {
        {"%1", colourSpaceInfo[from].funcName},
        {"%2", colourSpaceInfo[to].funcName},
    });
    program->addShaderFromSourceCode(QOpenGLShader::Compute, compSrc);

    program->link();
    return program;
}

Colour ColourConversionProgram::convert(const Colour &colour) {
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    Colour storageData = colour;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), &storageData, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);

    return storageData;
}

QOpenGLShaderProgram *ColourPickProgram::createProgram() const
{
    QOpenGLShaderProgram *const program = new QOpenGLShaderProgram();

    QString compSrc;
    compSrc += RenderManager::headerShaderPart();
    compSrc += RenderManager::bufferShaderPart("src", 0, 0, 0, format, indexed, 1, paletteFormat);
    compSrc +=
R"(
uniform layout(location = 2) vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
};

layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = src(pos);
}
)";
    program->addShaderFromSourceCode(QOpenGLShader::Compute, compSrc);

    program->link();
    return program;
}

Colour ColourPickProgram::pick(const Buffer *const src, const Buffer *const srcPalette, const QVector2D pos)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniform2f(2, static_cast<GLfloat>(pos.x()), static_cast<GLfloat>(pos.y()));
    qApp->renderManager.bindIndexedBufferShaderPart(program, "src", 0, 0, src, indexed, 1, 1, srcPalette);

    Colour storageData;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), nullptr, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);

    return storageData;
}

} // namespace GfxPaint
