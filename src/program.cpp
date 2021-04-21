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

QString EllipseProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(AttributelessModel::ClipQuad);
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
        src += R"(
uniform mat4 matrix;

uniform ivec2 destRectPos;
uniform ivec2 destRectSize;

uniform Rgba rgba;
uniform Index index;

bool inside(const vec2 pos) {
    float dist = length(pos);
    return bool(step(dist, 1.0));
}

bool pixelInside(const ivec2 pixel) {
    vec2 pos = (vec2(pixel - destRectPos) + vec2(0.5)) / vec2(destRectSize) * vec2(2.0) - vec2(1.0);
    return inside(pos);
}

Colour src(const vec2 pos) {
//    ivec2 pixel = ivec2(floor(gl_FragCoord.xy));
    // Is this accurate for both positive and negative offsets?
    vec2 dx = dFdx(pos);
    vec2 dy = dFdy(pos);
    bool isEdge = (inside(pos) && (!inside(pos - dx) || !inside(pos + dx) || !inside(pos - dy) || !inside(pos + dy)));
    return isEdge ? Colour(rgba, index) : Colour(Rgba(0.0, 0.0, 0.0, 0.0), INDEX_INVALID);
//    ivec2 pixel = ivec2(floor((pos + vec2(1.0)) / vec2(2.0) * vec2(destRectSize))) + destRectPos;
//    bool isEdge = (pixelInside(pixel) && (!pixelInside(pixel + ivec2(-1, 0)) || !pixelInside(pixel + ivec2(1, 0)) || !pixelInside(pixel + ivec2(0, -1)) || !pixelInside(pixel + ivec2(0, 1))));
//    bool isInside = pixelInside(pixel);
//    return isEdge ? Colour(rgba, index) : (isInside ? Colour(Rgba(0.5, 0.5, 0.5, 0.5), INDEX_INVALID) : Colour(Rgba(0.0, 0.0, 0.0, 0.0), INDEX_INVALID));
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::fragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void EllipseProgram::render(const std::array<QVector2D, 2> &points, const Colour &colour, const QMatrix4x4 &transform, Buffer * const dest, const Buffer * const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    QMatrix4x4 pointsMatrix;
    const QVector2D &offset = QVector2D(floor(points[0].x()), floor(points[0].y()));
    const QVector2D &scale =  QVector2D(floor(points[1].x() - points[0].x()), floor(points[1].y() - points[0].y()));
    pointsMatrix.translate(offset.x(), offset.y());
    pointsMatrix.scale(scale.x(), scale.y());
    pointsMatrix.scale(0.5f, 0.5f);
    pointsMatrix.translate(1.0f, 1.0f);

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, (transform * pointsMatrix).data());

    glUniform2i(program.uniformLocation("destRectPos"), 0, 0);
    glUniform2i(program.uniformLocation("destRectSize"), dest->width(), dest->height());

    glUniform4fv(program.uniformLocation("rgba"), 1, colour.rgba.data());
    glUniform1ui(program.uniformLocation("index"), colour.index);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

QString LineProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += R"(
struct Point {
    vec2 pos;
    float width;
    float linePos;
    Colour colour;
};

layout(std430, binding = 0) buffer StorageData
{
    Point points[];
} storageData;

out VertexOut {
    vec2 pos;
    float width;
    float linePos;
    Rgba rgba;
    Index index;
} vOut;

void main(void)
{
    Point point = storageData.points[gl_InstanceID + gl_VertexID];
    vOut.pos = point.pos;
    vOut.width = point.width;
    vOut.linePos = point.linePos;
    vOut.rgba = point.colour.rgba;
    vOut.index = point.colour.index;
}
)";
    }break;
    case QOpenGLShader::Geometry: {
        src += RenderManager::headerShaderPart();
        src += R"(
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 16) out;

in VertexOut {
    vec2 pos;
    float width;
    float linePos;
    Rgba rgba;
    Index index;
} gIn[];

uniform mat4 matrix;

out GeometryOut {
    float linePos;
    float segmentPos;
    flat float segmentLength;
    Rgba rgba;
    flat Index index;
    flat Rgba colour[2];
    vec2 uv;
} gOut;

void createEndPoint(const vec2 point0, const vec2 vector0, const vec2 point1, const vec2 vector1)
{
    vec2 intersection;
    float t0, t1;
    bool hasIntersection = intersectRays(point0, vector0, point1, vector1, intersection, t0, t1);
    if (hasIntersection) {
        gl_Position = matrix * vec4(intersection, 0.0, 1.0);
//    gOut.linePos = gIn[1].linePos;
//    gOut.segmentPos = 0.0/*gOut.segmentLength * -startAU*/;
    }
    else {
        gl_Position = matrix * vec4(point0, 0.0, 1.0);
//    gOut.linePos = gIn[1].linePos;
//    gOut.segmentPos = 0.0/*gOut.segmentLength * -startAU*/;
    }
    EmitVertex();
}

void main(void)
{
    vec2 segmentVector = gIn[2].pos - gIn[1].pos;
    vec2 segmentPerp = normalize(perp(segmentVector));
    gOut.segmentLength = length(segmentVector);
    gOut.colour = Rgba[](gIn[1].rgba, gIn[2].rgba);

    // Segment start
    vec2 startHalfWidthVector = segmentPerp * gIn[1].width / 2.0;
    vec2 startPointA = gIn[1].pos - startHalfWidthVector;
    vec2 startPointB = gIn[1].pos + startHalfWidthVector;

    vec2 prevSegmentVector = gIn[1].pos - gIn[0].pos;
    vec2 prevSegmentPerp = normalize(perp(prevSegmentVector));
    vec2 prevEndHalfWidthVector = prevSegmentPerp * gIn[1].width / 2.0;
    vec2 prevEndPointA = gIn[1].pos - prevEndHalfWidthVector;
    vec2 prevEndPointB = gIn[1].pos + prevEndHalfWidthVector;

    // Segment end
    vec2 endHalfWidthVector = segmentPerp * gIn[2].width / 2.0;
    vec2 endPointA = gIn[2].pos - endHalfWidthVector;
    vec2 endPointB = gIn[2].pos + endHalfWidthVector;

    vec2 nextSegmentVector = gIn[3].pos - gIn[2].pos;
    vec2 nextSegmentPerp = normalize(perp(nextSegmentVector));
    vec2 nextStartHalfWidthVector = nextSegmentPerp * gIn[2].width / 2.0;
    vec2 nextStartPointA = gIn[2].pos - nextStartHalfWidthVector;
    vec2 nextStartPointB = gIn[2].pos + nextStartHalfWidthVector;

    gOut.rgba = gIn[1].rgba;
    gOut.index = gIn[1].index;
    gOut.uv = vec2(0.0, 0.0);
    createEndPoint(startPointA, segmentVector, prevEndPointA, prevSegmentVector);
    gl_Position = matrix * vec4(gIn[1].pos, 0.0, 1.0);
    gOut.uv = vec2(0.0, 0.5);
    EmitVertex();
    gOut.rgba = gIn[2].rgba;
    gOut.index = gIn[2].index;
    gOut.uv = vec2(1.0, 0.0);
    createEndPoint(endPointA, segmentVector, nextStartPointA, nextSegmentVector);
    gl_Position = matrix * vec4(gIn[2].pos, 0.0, 1.0);
    gOut.uv = vec2(1.0, 0.5);
    EmitVertex();
    EndPrimitive();

    gOut.rgba = gIn[1].rgba;
    gOut.index = gIn[1].index;
    gOut.uv = vec2(0.0, 1.0);
    createEndPoint(startPointB, segmentVector, prevEndPointB, prevSegmentVector);
    gl_Position = matrix * vec4(gIn[1].pos, 0.0, 1.0);
    gOut.uv = vec2(0.0, 0.5);
    EmitVertex();
    gOut.rgba = gIn[2].rgba;
    gOut.index = gIn[2].index;
    gOut.uv = vec2(1.0, 1.0);
    createEndPoint(endPointB, segmentVector, nextStartPointB, nextSegmentVector);
    gl_Position = matrix * vec4(gIn[2].pos, 0.0, 1.0);
    gOut.uv = vec2(1.0, 0.5);
    EmitVertex();
    EndPrimitive();
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += R"(
in GeometryOut {
    float linePos;
    float segmentPos;
    flat float segmentLength;
    Rgba rgba;
    flat Index index;
    flat Rgba colour[2];
    vec2 uv;
} fIn;

uniform float lineLength;

Colour src(const vec2 pos) {
//    return Colour(int(fIn.segmentPos) % 10 < 5 ? fIn.rgba : Rgba(0.0, 0.0, 0.0, 1.0), fIn.index);
    return Colour(int(mod(fIn.uv.x, 0.5) < 0.25) + int(mod(fIn.uv.y, 0.5) < 0.25) == 1 ? mix(fIn.colour[0], fIn.colour[1], fIn.uv.x) : Rgba(0.0, 0.0, 0.0, 1.0), fIn.index);
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

    glBindVertexArray(vao);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(LineProgram::Point), points.data(), GL_STATIC_DRAW);

    glDrawArraysInstanced(GL_LINES_ADJACENCY, 0, 4, points.size() - 3);
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
