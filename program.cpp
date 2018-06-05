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
    vertSrc += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
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

    qApp->renderManager.bindBufferShaderPart(program, "src", 0, src);

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
    fragSrc += RenderManager::bufferShaderPart("src", 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
    fragSrc += RenderManager::bufferShaderPart("dest", 2, destFormat, destIndexed, 3, destPaletteFormat);
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 3, destPaletteFormat, blendMode, composeMode);
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

    qApp->renderManager.bindIndexedBufferShaderPart(program, "src", 0, src, srcIndexed, 1, srcPalette);

    if (dest) {
        qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 2, dest, destIndexed, 3, destPalette);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QOpenGLShaderProgram *ModelProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::modelVertexShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::modelFragmentShaderPart("src");
    fragSrc += RenderManager::bufferShaderPart("dest", 0, destFormat, destIndexed, 1, destPaletteFormat);
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void ModelProgram::render(Model *const model, const QColor &colour, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette) {
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    program.setUniformValue("matrix", transform);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    model->render();
}

QOpenGLShaderProgram *DabProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    QString vertSrc;
    vertSrc += RenderManager::headerShaderPart();
    vertSrc += RenderManager::attributelessShaderPart(type == Dab::Type::Pixel ? AttributelessModel::SingleVertex : AttributelessModel::ClipQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::dabShaderPart("src", type, metric);
    fragSrc += RenderManager::bufferShaderPart("dest", 0, destFormat, destIndexed, 1, destPaletteFormat);
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void DabProgram::render(const Dab &dab, const QColor &colour, const QTransform &transform, Buffer *const dest, const Buffer *const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    qTransformCopyToGLArray(dab.transform() * transform, uniformData.matrix);
    qColorCopyToGLArray(colour, uniformData.colour);
    uniformData.hardness = static_cast<GLfloat>(dab.hardness);
    uniformData.alpha = static_cast<GLfloat>(dab.opacity);

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);

    program.setUniformValue("matrix", dab.transform() * transform);

    glUniform4f(program.uniformLocation("srcColour"), static_cast<GLfloat>(colour.redF()), static_cast<GLfloat>(colour.greenF()), static_cast<GLfloat>(colour.blueF()), static_cast<GLfloat>(colour.alphaF()));
    glUniform1f(program.uniformLocation("srcHardness"), static_cast<GLfloat>(dab.hardness));
    glUniform1f(program.uniformLocation("srcOpacity"), static_cast<GLfloat>(dab.opacity));

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

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
    vertSrc += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
    vertSrc += RenderManager::vertexMainShaderPart();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc);

    QString fragSrc;
    fragSrc += RenderManager::headerShaderPart();
    fragSrc += RenderManager::colourSliderShaderPart("src", colourSpace, component, quantise, 1, quantisePaletteFormat);
    fragSrc += RenderManager::bufferShaderPart("dest", 0, destFormat, false, 0, Buffer::Format());
    fragSrc += RenderManager::fragmentMainShaderPart(destFormat, false, 1, Buffer::Format(), blendMode, 3);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);

    program->link();
    return program;
}

void ColourSliderProgram::render(const QColor &colour, const ColourSpace colourSpace, const int component, const QTransform &transform, Buffer *const dest, const Buffer *const quantisePalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    qColorCopyToGLArray(colour, uniformData.colour);
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_DYNAMIC_DRAW);

    program.setUniformValue("matrix", transform);

    qApp->renderManager.bindBufferShaderPart(program, "dest", 0, dest);
    if (quantise && quantisePalette) {
        qApp->renderManager.bindBufferShaderPart(program, "quantisePalette", 1, dest);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QOpenGLShaderProgram *ColourSliderPickProgram::createProgram() const
{
    QOpenGLShaderProgram *const program = new QOpenGLShaderProgram();

    QString compSrc;
    compSrc += RenderManager::headerShaderPart();
    compSrc += RenderManager::colourSliderShaderPart("src", colourSpace, component, quantise, 0, quantisePaletteFormat);
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

QColor ColourSliderPickProgram::pick(QColor colour, const float pos, const Buffer *const quantisePalette)
{
    typedef qreal (QColor::*const Getter)() const;
    static const Getter getters[4] = {&QColor::redF, &QColor::greenF, &QColor::blueF, &QColor::alphaF};
    for (int i = 0; i < 4; ++i) storageData.colour[i] = static_cast<GLfloat>((colour.*getters[i])());
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniform1f(program.uniformLocation("pos"), static_cast<GLfloat>(pos));
    if (quantise && quantisePalette) {
        qApp->renderManager.bindBufferShaderPart(program, "quantisePalette", 0, quantisePalette);
    }

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
    for (int i = 0; i < 4; ++i) (colour.*setters[i])(static_cast<qreal>(clamp(0.0f, 1.0f, storageData.colour[i])));
    return colour;
}

QOpenGLShaderProgram *ColourPlaneProgram::createProgram() const
{
}

void ColourPlaneProgram::render(const QColor &colour, const ColourSpace colourSpace, const int componentX, const int componentY, const QTransform &transform, Buffer *const dest)
{
}

QOpenGLShaderProgram *PatternProgram::createProgram() const
{
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/background.vert");

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
)";
    if (from != to) compSrc +=
R"(
    colour = vec4(%1_to_%2(colour.rgb), colour.a);
)";
    compSrc +=
R"(
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

void ColourConversionProgram::convert(const float from[], float to[]) {
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    memcpy(&storageData, from, sizeof(storageData));
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
    compSrc += RenderManager::bufferShaderPart("src", 0, format, indexed, 1, paletteFormat);
    compSrc +=
R"(
uniform layout(location = 3) vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    vec4 rgba;
    uint index;
};
)";
    if (indexed) compSrc +=
R"(
)";
    compSrc +=
R"(
layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    Colour colour = src(pos);
)";
    if (indexed) compSrc +=
R"(
    index = colour.index;
//    rgba = vec4(texelFetch(srcPaletteTexture, ivec2(index, 0)));
    rgba = srcPalette(index);
)";
    else compSrc +=
R"(
    rgba = colour.rgba;
    index = colour.index;
)";
    compSrc +=
R"(
    }
)";
    program->addShaderFromSourceCode(QOpenGLShader::Compute, compSrc);

    program->link();
    return program;
}

QColor ColourPickProgram::pick(const Buffer *const src, const Buffer *const srcPalette, const QPointF pos, uint *const index)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniform2f(program.uniformLocation("pos"), static_cast<GLfloat>(pos.x()), static_cast<GLfloat>(pos.y()));
    qApp->renderManager.bindIndexedBufferShaderPart(program, "src", 0, src, indexed, 1, srcPalette);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, storageBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(StorageData), nullptr, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(StorageData), &storageData);
    QColor colour;
    qColorFillFromGLArray(colour, storageData.rgba);
    if (index) *index = (indexed ? storageData.index : -1);
    return colour;
}

} // namespace GfxPaint
