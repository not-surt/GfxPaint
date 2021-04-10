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

QOpenGLShaderProgram *Program::createProgram() const
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    for (auto stage : programStages) {
        const QString src = generateSource(stage);
        if (!src.isEmpty()) {
            program->addShaderFromSourceCode(stage, src);
        }
    }

    program->link();
    Q_ASSERT(program->isLinked());

    return program;

}

QString RenderedWidgetProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
        src += RenderManager::vertexMainShaderPart();
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::bufferShaderPart("src", 0, 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
        src += RenderManager::widgetOutputShaderPart();
    }break;
    default: break;
    }

    return src;
}

void RenderedWidgetProgram::render(Buffer *const src, const QMatrix4x4 &transform)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, (qApp->renderManager.flipTransform * qApp->renderManager.unitToClipTransform/* * transform*/).data());

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), src->width(), src->height());

    qApp->renderManager.bindBufferShaderPart(program, "src", 0, src);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QString BufferProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
        src += RenderManager::vertexMainShaderPart();
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::bufferShaderPart("src", 0, 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
        src += RenderManager::bufferShaderPart("dest", 2, 2, destFormat, destIndexed, 3, destPaletteFormat);
        src += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 3, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void BufferProgram::render(Buffer *const src, const Buffer *const srcPalette, const Colour &srcTransparent, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette, const Colour &destTransparent)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    QMatrix4x4 matrix;
    matrix.scale(src->width(), src->height());
    matrix = transform * matrix;

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, matrix.data());

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), src->width(), src->height());

    glUniform4fv(program.uniformLocation("srcTransparent.colour"), 1, srcTransparent.rgba.data());
    glUniform1ui(program.uniformLocation("srcTransparent.index"), srcTransparent.index);
    glUniform4fv(program.uniformLocation("destTransparent.colour"), 1, destTransparent.rgba.data());
    glUniform1ui(program.uniformLocation("destTransparent.index"), destTransparent.index);

    UniformData uniformData;
    memcpy(&uniformData.matrix, matrix.data(), sizeof(uniformData.matrix));
    uniformData.transparent = srcTransparent;

    const GLuint blockIndex = glGetUniformBlockIndex(program.programId(), "srcUniformData");
    const GLuint bindingPoint = 2;
    glUniformBlockBinding(program.programId(), blockIndex, bindingPoint);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "src", 0, src, srcIndexed, 1, srcPalette);

    if (dest) {
        qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 2, dest, destIndexed, 3, destPalette);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QString ModelProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::modelVertexMainShaderPart();
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::modelFragmentShaderPart("src");
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void ModelProgram::render(Model *const model, const Colour &colour, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette) {
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.data());

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), 1, 1);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    model->render();
}

QString DabProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(type == Brush::Dab::Type::Pixel ? AttributelessModel::SingleVertex : AttributelessModel::ClipQuad);
        src += RenderManager::vertexMainShaderPart();
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::dabShaderPart("src", type, metric);
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }


    return src;
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

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), 1, 1);

    glUniform4fv(program.uniformLocation("srcColour"), 1, colour.rgba.data());
    glUniform1f(program.uniformLocation("srcHardness"), static_cast<GLfloat>(dab.hardness));
    glUniform1f(program.uniformLocation("srcOpacity"), static_cast<GLfloat>(dab.opacity));

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    if (dab.type == Brush::Dab::Type::Pixel) {
        glDrawArrays(GL_POINTS, 4, 1);
    }
    else {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    //glTextureBarrier();
}

QString PatternProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += fileToString(":/shaders/background.vert");
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::patternShaderPart("src", Pattern::Checkers);
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, false, 0, Buffer::Format());
        //src += RenderManager::fragmentMainShaderPart(destFormat);
        src += RenderManager::widgetOutputShaderPart();
    }break;
    default: break;
    }

    return src;
}

void PatternProgram::render(const QMatrix4x4 &transform)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();
    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.inverted().data());

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

QString ColourPlaneProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
        src += RenderManager::vertexMainShaderPart();
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::colourPlaneShaderPart("src", colourSpace, useXAxis, useYAxis, quantise, 1, quantisePaletteFormat);
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, false, 0, Buffer::Format());
        src += RenderManager::fragmentMainShaderPart(destFormat, false, 0, Buffer::Format(), blendMode, RenderManager::composeModeDefault);
    }break;
    default: break;
    }

    return src;
}

void ColourPlaneProgram::render(const Colour &colour, const int xComponent, const int yComponent, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const quantisePalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    UniformData uniformData = {colour, {xComponent, yComponent}};
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.data());

    glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("srcRectSize"), 1, 1);

    qApp->renderManager.bindBufferShaderPart(program, "dest", 0, dest);
    if (quantise && quantisePalette) {
        qApp->renderManager.bindBufferShaderPart(program, "quantisePalette", 1, quantisePalette);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //glTextureBarrier();
}

QString ColourPlanePickProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Compute: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::colourPlaneShaderPart("src", colourSpace, useXAxis, useYAxis, quantise, 0, quantisePaletteFormat);
        src +=
R"(
uniform layout(location = 2) vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
};

layout(local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = src(pos);
}
)";
    }break;
    default: break;
    }

    return src;
}

Colour ColourPlanePickProgram::pick(const Colour &colour, const QVector2D &pos, const Buffer *const quantisePalette)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    UniformData uniformData = {colour};
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    glUniform2fv(2, 1, (GLfloat *)&pos);
    if (quantise && quantisePalette) {
        qApp->renderManager.bindBufferShaderPart(program, "quantisePalette", 0, quantisePalette);
    }

    Colour storageData = colour;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), &storageData, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);
    void *mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), GL_MAP_READ_BIT);
    memcpy(&storageData, mapping, sizeof(storageData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    qDebug() << "PICK!" << storageData;///////////////////////////////////////////
    return storageData;
}

QString ColourPaletteProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(AttributelessModel::UnitQuad);
        src += RenderManager::vertexMainShaderPart();
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        if (paletteFormat.isValid()) {
            src += fileToString(":/shaders/palette.glsl");
            src += RenderManager::paletteShaderPart("srcPalette", 0, paletteFormat);
        }
        src += RenderManager::colourPaletteShaderPart("src");
        src += RenderManager::bufferShaderPart("dest", 1, 1, destFormat, false, 0, Buffer::Format());
        src += RenderManager::fragmentMainShaderPart(destFormat, false, 0, Buffer::Format(), blendMode, RenderManager::composeModeDefault);
    }break;
    default: break;
    }

    return src;
}

void ColourPaletteProgram::render(const Buffer *const palette, const QSize size, const QSize swatchSize, const QSize cells, const QMatrix4x4 &transform, Buffer *const dest)
{
    if (palette) {
        Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

        QOpenGLShaderProgram &program = this->program();
        program.bind();

        glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.data());

        const QSize size2 = QSize(cells.width() * swatchSize.width(), cells.height() * swatchSize.height());
        glUniform2i(program.uniformLocation("srcRectPos"), 0, 0);
        glUniform2i(program.uniformLocation("srcRectSize"), size2.width(), size2.height());

        glUniform2i(program.uniformLocation("cells"), 16, 16);
        glUniform2i(program.uniformLocation("swatchSize"), swatchSize.width(), swatchSize.height());

        qApp->renderManager.bindBufferShaderPart(program, "srcPalette", 0, palette);
        qApp->renderManager.bindBufferShaderPart(program, "dest", 1, dest);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}

QString ColourPalettePickProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Compute: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::bufferShaderPart("src", 0, 0, format, false, 0, Buffer::Format());
        src +=
R"(
uniform layout(location = 1) vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
};
layout(std140, binding = 1) uniform uniformData {
    mat4 matrix;
};

layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = src(pos);
}
)";
    }break;
    default: break;
    }

    return src;
}

Colour ColourPalettePickProgram::pick(const Buffer *const src, const QVector2D &pos, const QMatrix4x4 &transform)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    UniformData uniformData;
    memcpy(&uniformData.matrix, transform.data(), sizeof(mat4));
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    glUniform2fv(2, 1, (GLfloat *)&pos);
//    glUniform2f(2, pos.x(), pos.y());
    qApp->renderManager.bindIndexedBufferShaderPart(program, "src", 0, src, false, 0, nullptr);

    Colour storageData;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), &storageData, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);
    void *mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), GL_MAP_READ_BIT);
    memcpy(&storageData, mapping, sizeof(storageData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return storageData;
}

QString ColourConversionProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Compute: {
        src += RenderManager::headerShaderPart();
        src += fileToString(":/shaders/ColorSpaces.inc.glsl");
        src +=
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
        stringMultiReplace(src, {
            {"%1", colourSpaceInfo[from].funcName},
            {"%2", colourSpaceInfo[to].funcName},
        });
    }break;
    default: break;
    }

    return src;
}

Colour ColourConversionProgram::convert(const Colour &colour) {
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    Colour storageData = colour;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), &storageData, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);
    void *mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), GL_MAP_READ_BIT);
    if (mapping) { // Why sometimes null?
        memcpy(&storageData, mapping, sizeof(storageData));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return storageData;
}

QString ColourPickProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Compute: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::bufferShaderPart("dest", 0, 0, format, indexed, 1, paletteFormat);
        src +=
R"(
uniform layout(location = 2) vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
};

layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = dest(pos);
}
)";
    }break;
    default: break;
    }

    return src;
}

Colour ColourPickProgram::pick(const Buffer *const dest, const Buffer *const destPalette, const QVector2D &pos)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniform2fv(2, 1, (GLfloat *)&pos);
//    glUniform2f(2, pos.x(), pos.y());
    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, indexed, 1, destPalette);

    Colour storageData;
    storageData = Colour{Rgba{1.0f, 1.0f, 1.0f, 1.0f}, 1};
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), nullptr, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);
    void *mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), GL_MAP_READ_BIT);
    memcpy(&storageData, mapping, sizeof(storageData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return storageData;
}

} // namespace GfxPaint
