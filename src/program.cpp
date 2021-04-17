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
        src +=
            R"(
uniform mat4 matrix;

in layout(location = 0) vec2 vertexPos;
in layout(location = 1) vec4 vertexColour;

out layout(location = 0) vec2 pos;
out layout(location = 1) vec4 colour;

void main(void) {
    pos = vec2(0.0, 0.0);
    colour = vertexColour;
    gl_Position = matrix * vec4(vertexPos, 0.0, 1.0);
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src +=
        R"(
//in layout(location = 0) vec2 pos;
in layout(location = 1) vec4 colour;

Colour src(const vec2 pos) {
    return Colour(colour, INDEX_INVALID);
}
)";
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

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    model->render();
}

QString LineProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += R"(
layout(location = 0) in vec2 pos;
layout(location = 1) in float width;
layout(location = 2) in float lineRelPos;
layout(location = 3) in float lineAbsPos;
layout(location = 4) in Rgba rgba;
layout(location = 5) in Index index;

struct Point {
    vec2 pos;
    float width;
    float lineRelPos;
    float lineAbsPos;
    Colour colour;
};

layout(std430, binding = 0) buffer StorageData
{
    Point points[];
} storageData;

out vec2 geomPos;
out float geomWidth;
out float geomLineRelPos;
out float geomLineAbsPos;
out Rgba geomRgba;
out Index geomIndex;

void main(void)
{
    Point point = storageData.points[gl_InstanceID + gl_VertexID];
    geomPos = point.pos;
    geomWidth = point.width;
    geomLineRelPos = point.lineRelPos;
    geomLineAbsPos = point.lineAbsPos;
    geomRgba = point.colour.rgba;
    geomIndex = point.colour.index;
}
)";
    }break;
    case QOpenGLShader::Geometry: {
        src += RenderManager::headerShaderPart();
        src += R"(
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 8) out;

in vec2 geomPos[];
in float geomWidth[];
in float geomLineRelPos[];
in float geomLineAbsPos[];
in Rgba geomRgba[];
in Index geomIndex[];

uniform mat4 matrix;

out float fragLineRelPos;
out float fragLineAbsPos;
out float fragSegmentRelPos;
out float fragSegmentAbsPos;
out Rgba fragRgba;
out Index fragIndex;

void main(void)
{
    vec2 segmentVector = geomPos[2] - geomPos[1];
    float segmentLength = length(segmentVector);
    vec2 segmentPerp = normalize(perp(segmentVector));

    // Segment start
    fragLineRelPos = geomLineRelPos[1];
    fragLineAbsPos = geomLineAbsPos[1];
    fragSegmentAbsPos = 0.0;
    fragSegmentRelPos = 0.0;
    fragRgba = geomRgba[1];
    fragIndex = geomIndex[1];
    vec2 startHalfWidthVector = segmentPerp * geomWidth[1] / 2.0;
    gl_Position = matrix * vec4(geomPos[1] - startHalfWidthVector, 0.0, 1.0);
    EmitVertex();
    gl_Position = matrix * vec4(geomPos[1] + startHalfWidthVector, 0.0, 1.0);
    EmitVertex();

    // Segment end
    fragLineRelPos = geomLineRelPos[2];
    fragLineAbsPos = geomLineAbsPos[2];
    fragSegmentAbsPos = segmentLength;
    fragSegmentRelPos = 1.0;
    fragRgba = geomRgba[2];
    fragIndex = geomIndex[2];
    vec2 endHalfWidthVector = segmentPerp * geomWidth[2] / 2.0;
    gl_Position = matrix * vec4(geomPos[2] - endHalfWidthVector, 0.0, 1.0);
    EmitVertex();
    gl_Position = matrix * vec4(geomPos[2] + endHalfWidthVector, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();

//    // Connect to previous segment
//    if (gl_InstanceID != 0) {
//        vec2 prevSegmentVector = geomPos[1] - geomPos[0];
//        vec2 prevSegmentPerp = normalize(perp(prevSegmentVector));
//        vec2 avgPerp = normalize(segmentPerp + prevSegmentPerp);

//        fragLineRelPos = geomLineRelPos[1];
//        fragLineAbsPos = geomLineAbsPos[1];
//        fragSegmentAbsPos = 0.0;
//        fragSegmentRelPos = 0.0;
//        fragRgba = geomRgba[1];
//        fragIndex = geomIndex[1];

//        // Previous segment end
//        vec2 prevEndHalfWidthVector = prevSegmentPerp * geomWidth[1] / 2.0;
//        gl_Position = matrix * vec4(geomPos[1] - prevEndHalfWidthVector, 0.0, 1.0);
//        EmitVertex();
//        gl_Position = matrix * vec4(geomPos[1] + prevEndHalfWidthVector, 0.0, 1.0);
//        EmitVertex();

//        // current segment start
//        gl_Position = matrix * vec4(geomPos[1] - startHalfWidthVector, 0.0, 1.0);
//        EmitVertex();
//        gl_Position = matrix * vec4(geomPos[1] + startHalfWidthVector, 0.0, 1.0);
//        EmitVertex();
//        EndPrimitive();
//    }
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += R"(
in float fragLineRelPos;
in float fragLineAbsPos;
in float fragSegmentRelPos;
in float fragSegmentAbsPos;
in Rgba fragRgba;
in flat Index fragIndex;

Colour src(const vec2 pos) {
    return Colour(int(fragSegmentAbsPos) % 10 < 5 ? fragRgba : Rgba(0.0, 0.0, 0.0, 1.0), fragIndex);
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void LineProgram::render(const std::vector<LineProgram::Point> &points, const Colour &colour, const QMatrix4x4 &transform, Buffer *const dest, const Buffer *const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, transform.data());

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    GLuint vao;
    GLuint vertexBuffer;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertexBuffer);

    glBindVertexArray(vao);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(LineProgram::Point), points.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(LineProgram::Point), points.data(), GL_STATIC_DRAW);

    struct Attrib {
        GLint size;
        GLenum type;
        size_t offset;
    };
    const std::vector<Attrib> attribs{
        {2, GL_FLOAT, 16},
        {1, GL_FLOAT, 16},
        {1, GL_FLOAT, 16},
        {1, GL_FLOAT, 16},
        {4, GL_FLOAT, 16},
        {1, GL_UNSIGNED_INT, 16},
    };
    const size_t stride = sizeof(LineProgram::Point);
    for (size_t i = 0u, offset = 0u; i < attribs.size(); offset += attribs[i].offset, ++i) {
        glVertexAttribPointer(i, attribs[i].size, attribs[i].type, false, stride, (GLvoid *)offset);
        glVertexAttribDivisor(i, 0);
        glVertexBindingDivisor(i, 0);
        glEnableVertexAttribArray(i);
    }

//    for (size_t i = 0; i < points.size() - 3; ++i) {
//        glDrawArrays(GL_LINES_ADJACENCY, i, 4);
//    }
    glDrawArraysInstanced(GL_LINES_ADJACENCY, 0, 4, points.size() - 3);

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
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

QString ColourPaletteProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(AttributelessModel::ClipQuad);
//        src += RenderManager::vertexMainShaderPart();
        src += R"(
uniform mat4 matrix;

out layout(location = 0) vec2 pos;

void main(void) {
    vec2 vertexPos = vertices[gl_VertexID];
    pos = vertexPos;
    gl_Position = matrix * vec4(vertexPos, 0.0, 1.0);
}
)";
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

void ColourPaletteProgram::render(const Buffer *const palette, const QSize &cells, const QMatrix4x4 &transform, Buffer *const dest)
{
    if (palette) {
        Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

        QOpenGLShaderProgram &program = this->program();
        program.bind();

        glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, RenderManager::unitToClipTransform.data());
        glUniform2i(program.uniformLocation("cells"), cells.width(), cells.height());

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
        src += fileToString(":/shaders/palette.glsl");
        src += RenderManager::paletteShaderPart("srcPalette", 0, format);
        src += RenderManager::colourPaletteShaderPart("src");
        src +=
R"(
uniform vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
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

Colour ColourPalettePickProgram::pick(const Buffer *const palette, const QSize &cells, const QVector2D &pos)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, RenderManager::unitToClipTransform.data());
    glUniform2i(program.uniformLocation("cells"), cells.width(), cells.height());

    qApp->renderManager.bindBufferShaderPart(program, "srcPalette", 0, palette);

//    glUniform2fv(program.uniformLocation("pos"), 1, (GLfloat *)&pos);
    glUniform2f(program.uniformLocation("pos"), pos.x(), pos.y());

    StorageData storageData;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), &storageData, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);
    void *mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), GL_MAP_READ_BIT);
    memcpy(&storageData, mapping, sizeof(storageData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return storageData.colour;
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

    StorageData storageData;
    storageData.colour = colour;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), &storageData, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);
    void *mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), GL_MAP_READ_BIT);
    Q_ASSERT(mapping != nullptr); // Why sometimes null?
    memcpy(&storageData, mapping, sizeof(storageData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return storageData.colour;
}

QString ColourPickProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Compute: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::bufferShaderPart("src", 0, 0, format, indexed, 1, paletteFormat);
        src +=
R"(
uniform vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
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

Colour ColourPickProgram::pick(const Buffer *const src, const Buffer *const srcPalette, const QVector2D &pos)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    //    glUniform2f(2, pos.x(), pos.y());
    //    glUniform2fv(2, 1, (GLfloat *)&pos);
    //    glUniform2f(2, pos.x(), pos.y());
    glUniform2f(program.uniformLocation("pos"), pos.x(), pos.y());
    //    glUniform2f(program.uniformLocation("pos"), 40, 5);
    qApp->renderManager.bindIndexedBufferShaderPart(program, "src", 0, src, indexed, 1, srcPalette);

    StorageData storageData;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(storageData), &storageData, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    //    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), &storageData);
    void *mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(storageData), GL_MAP_READ_BIT);
    memcpy(&storageData, mapping, sizeof(storageData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    // TODO: paint colour glitchy with valid index
    storageData.colour.index = INDEX_INVALID;//////////////////////////////////////////
    return storageData.colour;
}

} // namespace GfxPaint
