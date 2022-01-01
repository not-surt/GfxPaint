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
    const QString &src = RenderManager::resourceShaderPart("fragment.glsl");
    const QString &preprocessed = qApp->renderManager.preprocessGlsl(src);
    qDebug().noquote() << "PREPROCESSED:" << preprocessed;

    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

    for (auto stage : programStages) {
        const QString src = generateSource(stage);
        if (!src.isEmpty()) {
            program->addCacheableShaderFromSourceCode(stage, src);
        }
    }

    program->link();
    Q_ASSERT_X(program->isLinked(), typeid(*this).name(), "Program linking failed.");

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
        src += RenderManager::bufferShaderPart("srcBuffer", 0, 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
        src += RenderManager::standardInputFragmentShaderPart("srcBuffer");
        src += RenderManager::widgetFragmentMainShaderPart();
    }break;
    default: break;
    }

    return src;
}

void RenderedWidgetProgram::render(Buffer *const src, const Mat4 &worldToClip)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    Mat4 objectMatrix;
    objectMatrix.scale(src->width(), src->height());
    glUniformMatrix4fv(program.uniformLocation("object"), 1, false, objectMatrix.constData());

    Mat4 widgetMatrix = qApp->renderManager.flipTransform * qApp->renderManager.unitToClipTransform * objectMatrix.inverted();
    glUniformMatrix4fv(program.uniformLocation("transform"), 1, false, widgetMatrix.constData());

    qApp->renderManager.bindBufferShaderPart(program, "srcBuffer", 0, src);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
        src += RenderManager::bufferShaderPart("srcBuffer", 0, 0, srcFormat, srcIndexed, 1, srcPaletteFormat);
        src += RenderManager::standardInputFragmentShaderPart("srcBuffer");
        src += RenderManager::bufferShaderPart("dest", 2, 2, destFormat, destIndexed, 3, destPaletteFormat);
        src += RenderManager::standardFragmentMainShaderPart(destFormat, destIndexed, 3, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void BufferProgram::render(Buffer *const src, const Buffer *const srcPalette, const Colour &srcTransparent, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette, const Colour &destTransparent)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    Mat4 objectMatrix;
    objectMatrix.scale(src->width(), src->height());
    glUniformMatrix4fv(program.uniformLocation("object"), 1, false, objectMatrix.constData());

    glUniformMatrix4fv(program.uniformLocation("transform"), 1, false, worldToClip.constData());

    glUniform4fv(program.uniformLocation("srcTransparent.colour"), 1, srcTransparent.rgba.data());
    glUniform1ui(program.uniformLocation("srcTransparent.index"), srcTransparent.index);
    glUniform4fv(program.uniformLocation("destTransparent.colour"), 1, destTransparent.rgba.data());
    glUniform1ui(program.uniformLocation("destTransparent.index"), destTransparent.index);

//    UniformData uniformData;
//    memcpy(&uniformData.matrix, matrix.constData(), sizeof(uniformData.matrix));
//    uniformData.transparent = srcTransparent;

//    const GLuint blockIndex = glGetUniformBlockIndex(program.programId(), "srcUniformData");
//    const GLuint bindingPoint = 2;
//    glUniformBlockBinding(program.programId(), blockIndex, bindingPoint);
//    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer);
//    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "srcBuffer", 0, src, srcIndexed, 1, srcPalette);

    if (dest) {
        qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 2, dest, destIndexed, 3, destPalette);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //glTextureBarrier();
}

QString SingleColourModelProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += R"(
uniform Rgba colour;
uniform mat4 worldToClip;

in layout(location = 0) vec2 pos;

// Unused to stop complaining about unmatching input in frag
out layout(location = 0) vec2 pos2;

out VertexOut {
    vec4 colour;
} vOut;

void main(void) {
    vOut.colour = colour;
    gl_Position = worldToClip * vec4(pos, 0.0, 1.0);
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += R"(
in VertexOut {
    vec4 colour;
} fIn;

Colour src(void) {
    return Colour(fIn.colour, INDEX_INVALID);
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::standardFragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void SingleColourModelProgram::render(Model *const model, const Colour &colour, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette) {
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniform4fv(program.uniformLocation("colour"), 1, colour.rgba.data());
    glUniformMatrix4fv(program.uniformLocation("worldToClip"), 1, false, worldToClip.constData());

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    model->render();
}

QString VertexColourModelProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src +=
            R"(
uniform mat4 worldToClip;

in layout(location = 0) vec2 vertexPos;
in layout(location = 1) vec4 vertexColour;

out layout(location = 0) vec2 pos;
out layout(location = 1) vec4 colour;

void main(void) {
    pos = vec2(0.0, 0.0);
    colour = vertexColour;
    gl_Position = worldToClip * vec4(vertexPos, 0.0, 1.0);
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src +=
            R"(
in layout(location = 0) vec2 pos;
in layout(location = 1) vec4 colour;

Colour src(void) {
    return Colour(colour, INDEX_INVALID);
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::standardFragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void VertexColourModelProgram::render(Model *const model, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette) {
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("worldToClip"), 1, false, worldToClip.constData());

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    model->render();
}

void BoundedPrimitiveProgram::render(const std::array<Vec2, 2> &points, const Colour &colour, const Mat4 &toolSpaceTransform, const Mat4 &worldToClip, Buffer * const dest, const Buffer * const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    // TODO: Do geometry transform in different spaces then final transform? Like with dabs.
    bool snap = false;
    std::array<Vec2, 2> processedPoints;
    for (std::size_t  i = 0; i < points.size(); ++i) {
        processedPoints[i] = toolSpaceTransform * points[i];
        if (snap) {
            processedPoints[i] = Vec2(processedPoints[i] + Vec2(0.5)).floor();
        }
    }
    Mat4 pointsMatrix;
    const Vec2 &offset = processedPoints[0];
    const Vec2 &scale = processedPoints[1] - processedPoints[0];
    pointsMatrix.translate(offset.x(), offset.y());
    pointsMatrix.scale(scale.x(), scale.y());
    pointsMatrix.scale(0.5f, 0.5f);
    pointsMatrix.translate(1.0f, 1.0f);

    // worldToTool -> toolToBuffer -> bufferToClip ?
    // or
    // worldToBuffer -> bufferToTool -> toolToBuffer (inverted) -> BufferToClip ? When no tool space can still use same other matrices
    glUniformMatrix4fv(program.uniformLocation("worldToBuffer"), 1, false, Mat4().constData());
    glUniformMatrix4fv(program.uniformLocation("bufferToTool"), 1, false, Mat4().constData());
    glUniformMatrix4fv(program.uniformLocation("BufferToClip"), 1, false, Mat4().constData());

    glUniformMatrix4fv(program.uniformLocation("matrix"), 1, false, (worldToClip * toolSpaceTransform.inverted() *  pointsMatrix).constData());

    glUniform4fv(program.uniformLocation("rgba"), 1, colour.rgba.data());
    glUniform1ui(program.uniformLocation("index"), colour.index);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

QString BoundedDistancePrimitiveProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
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
//    case QOpenGLShader::Geometry: {
//        src += RenderManager::headerShaderPart();
//        src += R"(
//)";
//    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += generateDistanceSource();
        src += R"(
in layout(location = 0) vec2 pos;

uniform mat4 matrix;

uniform Rgba rgba;
uniform Index index;

bool inside(const vec2 pos) {
    return dist(pos) < 1.0;
}

Colour src(void) {
)";
        if (filled) src += R"(
    return inside(pos) ? Colour(rgba, index) : Colour(Rgba(0.0, 0.0, 0.0, 0.0), INDEX_INVALID);
)";
        else src += R"(
    // Is this accurate for both positive and negative offsets?
    vec2 dx = dFdx(pos);
    vec2 dy = dFdy(pos);
    bool isEdge = (inside(pos) && (!inside(pos - dx) || !inside(pos + dx) || !inside(pos - dy) || !inside(pos + dy)));
    return isEdge ? Colour(rgba, index) : Colour(Rgba(0.0, 0.0, 0.0, 0.0), INDEX_INVALID);
)";
        src += R"(
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::standardFragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

QString RectProgram::generateDistanceSource() const
{
    return R"(
float dist(const vec2 pos) {
    return max(abs(pos.x), abs(pos.y));
}
)";
}

QString EllipseProgram::generateDistanceSource() const
{
    return R"(
float dist(const vec2 pos) {
    return length(pos);
}
)";
}

QString ContourStencilProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    const QString common = R"(
struct Point {
    vec2 pos;
    float pressure;
    vec4 quaternion;
    float age;
    float distance;
};

layout(std430, binding = 0) buffer StorageData {
    Point points[];
} storageData;
)";

    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += common;
        src += R"(
out Point point;

void main(void)
{
    point = storageData.points[gl_InstanceID + gl_VertexID];
}
)";
    }break;
    case QOpenGLShader::Geometry: {
        src += RenderManager::headerShaderPart();
        src += common;
        src += R"(
layout (lines) in;
layout (triangle_strip, max_vertices = 3) out;

in Point point[];

uniform mat4 worldToClip;

void main(void)
{
    gl_Position = worldToClip * vec4(storageData.points[0].pos, 0.0, 1.0);
    EmitVertex();
    gl_Position = worldToClip * vec4(point[0].pos, 0.0, 1.0);
    EmitVertex();
    gl_Position = worldToClip * vec4(point[1].pos, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += R"(
//layout(location = 0) out vec4 rgba;

void main(void)
{
//    rgba = vec4(1.0);
}
)";
    }break;
    default: break;
    }

    return src;
}

void ContourStencilProgram::render(const std::vector<Stroke::Point> &points, const Mat4 &worldToClip, Buffer *const dest)
{
    if (points.size() < 2) return;

    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    glGenTextures(1, &stencilTexture);
    glBindTexture(GL_TEXTURE_2D, stencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, dest->width(), dest->height(), 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTexture, 0);

    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 0, 0x1u);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

    glStencilMask(0xffu);
    glClear(GL_STENCIL_BUFFER_BIT);
    // Draw stencil
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(Stroke::Point), points.data(), GL_STATIC_DRAW);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("worldToClip"), 1, false, worldToClip.constData());

    glDrawArraysInstanced(GL_LINES, 1, 2, points.size() - 2);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0x00u);
    glStencilFunc(GL_EQUAL, 1, 0x1u);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void ContourStencilProgram::postRender()
{
    glDisable(GL_STENCIL_TEST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    glDeleteTextures(1, &stencilTexture);
    stencilTexture = 0;
}

QString SmoothQuadProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    const QString common = R"(
struct Point {
    vec2 pos;
};
)";

    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += common;
        src += R"(
layout(std430, binding = 0) buffer StorageData
{
    Point points[];
} storageData;

uniform mat4 worldToClip;

void main(void)
{
    gl_Position = worldToClip * vec4(storageData.points[gl_VertexID].pos, 0.0, 1.0);
}
)";
    }break;
    case QOpenGLShader::Geometry: {
        src += RenderManager::headerShaderPart();
        src += common;
       src += R"(
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 clipToWindow;

out flat vec4 projected[4];

void main(void)
{
    for (uint i = 0u; i < 4u; ++i) {
        projected[i] = clipToWindow * gl_in[i].gl_Position;
    }
    uint vertOrder[4] = uint[](3u, 0u, 2u, 1u);
    for (uint i = 0u; i < 4u; ++i) {
        gl_Position = gl_in[vertOrder[i]].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += common;
        src += R"(
in flat vec4 projected[4];

Colour src(void) {
    uint checkerCount = 8u;
    float checkerSize = 1.0 / float(checkerCount);
    vec2 uv = inverseBilinear(gl_FragCoord.xy, projected[0].xy, projected[1].xy, projected[2].xy, projected[3].xy);
    bool alternate = !((mod(uv.x, 2.0 * checkerSize) >= checkerSize) == (mod(uv.y, 2.0 * checkerSize) >= checkerSize));
    Rgba rgba = (alternate ? Rgba(1.0, 0.0, 0.0, 1.0) : Rgba(0.0, 1.0, 0.0, 1.0)) * Rgba(uv, 1.0, 1.0);
    return Colour(rgba, INDEX_INVALID);
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::standardFragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void SmoothQuadProgram::render(const std::vector<vec2> &points, const Mat4 &worldToClip, Buffer * const dest, const Buffer * const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("worldToClip"), 1, false, worldToClip.constData());

    Mat4 clipToWindow = viewportToClipTransform(dest->size()).inverted();
    glUniformMatrix4fv(program.uniformLocation("clipToWindow"), 1, false, clipToWindow.constData());

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(Stroke::Point), points.data(), GL_STATIC_DRAW);

    glDrawArrays(GL_LINES_ADJACENCY, 0, 4);
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

uniform mat4 worldToClip;

// Unused to stop complaining about unmatching input in frag
out layout(location = 0) vec2 pos;

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
        gl_Position = worldToClip * vec4(intersection, 0.0, 1.0);
//    gOut.linePos = gIn[1].linePos;
//    gOut.segmentPos = 0.0/*gOut.segmentLength * -startAU*/;
    }
    else {
        gl_Position = worldToClip * vec4(point0, 0.0, 1.0);
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
    gl_Position = worldToClip * vec4(gIn[1].pos, 0.0, 1.0);
    gOut.uv = vec2(0.0, 0.5);
    EmitVertex();
    gOut.rgba = gIn[2].rgba;
    gOut.index = gIn[2].index;
    gOut.uv = vec2(1.0, 0.0);
    createEndPoint(endPointA, segmentVector, nextStartPointA, nextSegmentVector);
    gl_Position = worldToClip * vec4(gIn[2].pos, 0.0, 1.0);
    gOut.uv = vec2(1.0, 0.5);
    EmitVertex();
    EndPrimitive();

    gOut.rgba = gIn[1].rgba;
    gOut.index = gIn[1].index;
    gOut.uv = vec2(0.0, 1.0);
    createEndPoint(startPointB, segmentVector, prevEndPointB, prevSegmentVector);
    gl_Position = worldToClip * vec4(gIn[1].pos, 0.0, 1.0);
    gOut.uv = vec2(0.0, 0.5);
    EmitVertex();
    gOut.rgba = gIn[2].rgba;
    gOut.index = gIn[2].index;
    gOut.uv = vec2(1.0, 1.0);
    createEndPoint(endPointB, segmentVector, nextStartPointB, nextSegmentVector);
    gl_Position = worldToClip * vec4(gIn[2].pos, 0.0, 1.0);
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

Colour src(void) {
//    return Colour(int(fIn.segmentPos) % 10 < 5 ? fIn.rgba : Rgba(0.0, 0.0, 0.0, 1.0), fIn.index);
    return Colour(int(mod(fIn.uv.x, 0.5) < 0.25) + int(mod(fIn.uv.y, 0.5) < 0.25) == 1 ? mix(fIn.colour[0], fIn.colour[1], fIn.uv.x) : Rgba(0.0, 0.0, 0.0, 1.0), fIn.index);
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::standardFragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void LineProgram::render(const std::vector<LineProgram::Point> &points, const Colour &colour, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("worldToClip"), 1, false, worldToClip.constData());

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(LineProgram::Point), points.data(), GL_STATIC_DRAW);

    glDrawArraysInstanced(GL_LINES_ADJACENCY, 0, 4, points.size() - 3);
}

QString PixelLineProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    const QString common = R"(
struct Point {
    vec2 pos;
    float pressure;
    vec4 quaternion;
    float age;
    float distance;
};
)";

    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += common;
        src += R"(
uniform mat4 worldToBuffer;
uniform mat4 bufferToClip;

layout(std430, binding = 0) buffer StorageData {
    Point points[];
} storageData;

void main(void) {
    Point point = storageData.points[gl_InstanceID + gl_VertexID];
    vec2 bufferPos = (worldToBuffer * vec4(point.pos, 0.0, 1.0)).xy;
    vec2 snappedPos = snap(vec2(0.0), vec2(1.0), bufferPos) + vec2(0.5);
    gl_Position = bufferToClip * vec4(snappedPos, 0.0, 1.0);
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += common;
        src += R"(
uniform Colour colour;

Colour src(void) {
    return colour;
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::standardFragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void PixelLineProgram::render(const std::vector<Stroke::Point> &points, const Colour &colour, const Mat4 &worldToBuffer, const Mat4 &bufferToClip, Buffer *const dest, const Buffer *const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniformMatrix4fv(program.uniformLocation("worldToBuffer"), 1, false, worldToBuffer.constData());
    glUniformMatrix4fv(program.uniformLocation("bufferToClip"), 1, false, bufferToClip.constData());
    glUniform4fv(program.uniformLocation("colour.rgba"), 1, colour.rgba.data());
    glUniform1ui(program.uniformLocation("colour.index"), colour.index);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(Stroke::Point), points.data(), GL_STATIC_DRAW);

    glDrawArraysInstanced(GL_LINES, 0, 2, points.size() - 1);
    glDrawArrays(GL_POINTS, points.size() - 1, 1);
}

QString BrushDabProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    const QString common = R"(
struct Point {
    vec2 pos;
    float pressure;
    vec4 quaternion;
    float age;
    float distance;
};
layout(std430, binding = 0) buffer StorageData
{
    Point points[];
};

struct Dab {
    float hardness;
    float opacity;
};
layout(std140, binding = 0) uniform UniformData {
    uniform mat4 object;
    uniform mat4 worldToBuffer;
    uniform mat4 bufferToClip;
    Colour colour;
    Dab dab;
};
)";

    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += common;
        src += R"(
void main(void) {
    Point point = points[gl_InstanceID];
    gl_Position = bufferToClip * (worldToBuffer * vec4(point.pos, 0.0, 1.0));
}
)";
    }break;
    case QOpenGLShader::Geometry: {
        src += RenderManager::headerShaderPart();
        src += common;
        src += RenderManager::attributelessShaderPart(AttributelessModel::ClipQuad);
        src += R"(
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 pos;

void main(void)
{
    for (uint i = 0u; i < 4u; ++i) {
        pos = vertices[i];
        vec4 delta = bufferToClip * vec4(pos * 32.0, 0.0, 1.0) - bufferToClip * vec4(0.0, 0.0, 0.0, 1.0);
        gl_Position = gl_in[0].gl_Position + vec4(delta.xy, 0.0, 0.0);
        EmitVertex();
    }
    EndPrimitive();
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::resourceShaderPart("distance.glsl");
        src += common;
        src += R"(
in vec2 pos;

)";
        if (type == Brush::Dab::Type::Distance) {
            src += QString(R"(
float dabDistance(const vec2 pos) {
    return %1(pos);
}
)").arg(RenderManager::distanceMetrics[metric].functionName);
            src += R"(
Colour dabColour(const vec2 pos) {
    float weight;
    weight = clamp(1.0 - dabDistance(pos), 0.0, 1.0);
    weight = clamp(weight * (1.0 / (1.0 - dab.hardness)), 0.0, 1.0);
    weight *= dab.opacity;
    float alpha = colour.rgba.a * weight;
    uint index = INDEX_INVALID;
    if (alpha == colour.rgba.a) {
        index = colour.index;
    }
    return Colour(vec4(colour.rgba.rgb, alpha), INDEX_INVALID);
}
)";
        }
        else if (type == Brush::Dab::Type::Buffer) {
            src += R"(
float dabDistance(const vec2 pos) {
    return distanceChebyshev(pos);
}

Colour dabColour(const vec2 pos) {
    return COLOUR_INVALID;
}
)";
        }
        src += R"(
Colour src(void) {
    Colour colour = dabColour(pos);
    gl_FragDepth = pow(dabDistance(pos), 2.0);
//    if (gl_FragDepth >= 1.0) discard;
    return colour;
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, destIndexed, 1, destPaletteFormat);
        src += RenderManager::standardFragmentMainShaderPart(destFormat, destIndexed, 1, destPaletteFormat, blendMode, composeMode);
    }break;
    default: break;
    }

    return src;
}

void BrushDabProgram::render(const std::vector<Stroke::Point> &points, const Brush::Dab &dab, const Colour &colour, const Mat4 &worldToBuffer, const Mat4 &bufferToClip, Buffer *const dest, const Buffer *const destPalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    GLuint stencilTexture;
    glGenTextures(1, &stencilTexture);
    glBindTexture(GL_TEXTURE_2D, stencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, dest->width(), dest->height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, stencilTexture, 0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);
    glClearDepthf(1.0f);
    glDepthRangef(0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    struct alignas(16) Dab {
        GLfloat hardness;
        GLfloat opacity;
    };
    struct alignas(16) UniformData {
        mat4 object;
        mat4 worldToBuffer;
        mat4 bufferToClip;
        Colour colour;
        Dab dab;
    } uniformData{
        dab.transform().mat4(),
        worldToBuffer.mat4(),
        bufferToClip.mat4(),
        colour,
        {
             dab.hardness,
             dab.opacity,
        },
    };
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &uniformData, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, points.size() * sizeof(Stroke::Point), points.data(), GL_STATIC_DRAW);

    qApp->renderManager.bindIndexedBufferShaderPart(program, "dest", 0, dest, destIndexed, 1, destPalette);

    glDrawArraysInstanced(GL_POINTS, 0, 1, points.size());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);

    glDisable(GL_DEPTH_TEST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    glDeleteTextures(1, &stencilTexture);
}

QString BackgroundCheckersProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(AttributelessModel::ClipQuad);
        src += R"(
uniform mat4 worldToClip;

out layout(location = 0) vec2 pos;

void main(void) {
    vec2 vertexPos = vertices[gl_VertexID];
    pos = vec3(worldToClip * vec4(vertexPos, 0.0, 1.0)).xy;
    gl_Position = vec4(vertexPos, 0.0, 1.0);
}
)";
//        src += RenderManager::vertexMainShaderPart();
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::patternShaderPart("srcPattern", Pattern::Checkers);
        src += RenderManager::standardInputFragmentShaderPart("srcPattern");
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, false, 0, Buffer::Format());
        //src += RenderManager::standardFragmentMainShaderPart(destFormat);
        src += RenderManager::widgetFragmentMainShaderPart();
    }break;
    default: break;
    }

    return src;
}

void BackgroundCheckersProgram::render(const Mat4 &transform)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();
    glUniformMatrix4fv(program.uniformLocation("worldToClip"), 1, false, transform.inverted().constData());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
        src += RenderManager::colourPlaneShaderPart("srcPlane", colourSpace, useXAxis, useYAxis, quantise, 1, quantisePaletteFormat);
        src += R"(
in layout(location = 0) vec2 pos;

Colour src(void) {
    return srcPlane(pos);
}
)";
        src += RenderManager::bufferShaderPart("dest", 0, 0, destFormat, false, 0, Buffer::Format());
        src += RenderManager::standardFragmentMainShaderPart(destFormat, false, 0, Buffer::Format(), blendMode, RenderManager::composeModeDefault);
    }break;
    default: break;
    }

    return src;
}

void ColourPlaneProgram::render(const Colour &colour, const int xComponent, const int yComponent, const Mat4 &worldToClip, Buffer *const dest, const Buffer *const quantisePalette)
{
    Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

    QOpenGLShaderProgram &program = this->program();
    program.bind();

    UniformData uniformData = {colour, {xComponent, yComponent}};
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniformData), &uniformData, GL_DYNAMIC_DRAW);

    Mat4 objectMatrix;
    objectMatrix.scale(1.0, 1.0);
    glUniformMatrix4fv(program.uniformLocation("object"), 1, false, objectMatrix.constData());

    glUniformMatrix4fv(program.uniformLocation("transform"), 1, false, worldToClip.constData());

    qApp->renderManager.bindBufferShaderPart(program, "dest", 0, dest);
    if (quantise && quantisePalette) {
        qApp->renderManager.bindBufferShaderPart(program, "quantisePalette", 1, quantisePalette);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

QString ColourPaletteProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Vertex: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::attributelessShaderPart(AttributelessModel::ClipQuad);
        src += R"(
uniform mat4 worldToClip;

out layout(location = 0) vec2 pos;

void main(void) {
    vec2 vertexPos = vertices[gl_VertexID];
    pos = vertexPos;
    gl_Position = worldToClip * vec4(vertexPos, 0.0, 1.0);
}
)";
    }break;
    case QOpenGLShader::Fragment: {
        src += RenderManager::headerShaderPart();
        if (paletteFormat.isValid()) {
            src += RenderManager::resourceShaderPart("palette.glsl");
            src += RenderManager::paletteShaderPart("srcPalPalette", 0, paletteFormat);
        }
        src += RenderManager::colourPaletteShaderPart("srcPal");
        src += RenderManager::standardInputFragmentShaderPart("srcPal");
        src += RenderManager::bufferShaderPart("dest", 1, 1, destFormat, false, 0, Buffer::Format());
        src += RenderManager::standardFragmentMainShaderPart(destFormat, false, 0, Buffer::Format(), blendMode, RenderManager::composeModeDefault);
    }break;
    default: break;
    }

    return src;
}

void ColourPaletteProgram::render(const Buffer *const palette, const QSize &cells, const Mat4 &worldToClip, Buffer *const dest)
{
    if (palette) {
        Q_ASSERT(QOpenGLContext::currentContext() == &qApp->renderManager.context);

        QOpenGLShaderProgram &program = this->program();
        program.bind();

        glUniformMatrix4fv(program.uniformLocation("worldToClip"), 1, false, RenderManager::unitToClipTransform.constData());
        glUniform2i(program.uniformLocation("cells"), cells.width(), cells.height());

        qApp->renderManager.bindBufferShaderPart(program, "srcPalPalette", 0, palette);
        qApp->renderManager.bindBufferShaderPart(program, "dest", 1, dest);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

QString ColourPalettePickProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Compute: {
        src += RenderManager::headerShaderPart();
        src += fileToString(":/shaders/palette.glsl");
        src += RenderManager::paletteShaderPart("srcPalPalette", 0, format);
        src += RenderManager::colourPaletteShaderPart("srcPal");
        src +=
R"(
uniform vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
};

layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = srcPal(pos);
}
)";
    }break;
    default: break;
    }

    return src;
}

Colour ColourPalettePickProgram::pick(const Buffer *const palette, const QSize &cells, const Vec2 &pos)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniform2i(program.uniformLocation("cells"), cells.width(), cells.height());

    qApp->renderManager.bindBufferShaderPart(program, "srcPalPalette", 0, palette);

    glUniform2fv(program.uniformLocation("pos"), 1, (GLfloat *)&pos);
//    glUniform2f(program.uniformLocation("pos"), pos.x(), pos.y());

    Colour colour;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Colour), &colour, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    void *const mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Colour), GL_MAP_READ_BIT);
    memcpy(&colour, mapping, sizeof(Colour));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return colour;
}

QString ColourConversionProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Compute: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::resourceShaderPart("ColorSpaces.inc.glsl");
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

    Colour convert;
    convert = colour;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Colour), &convert, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    void *const mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Colour), GL_MAP_READ_BIT);
    // TODO: Why sometimes null?
    Q_ASSERT(mapping != nullptr);
    memcpy(&convert, mapping, sizeof(Colour));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return convert;
}

QString ColourPickProgram::generateSource(QOpenGLShader::ShaderTypeBit stage) const
{
    QString src;

    switch(stage) {
    case QOpenGLShader::Compute: {
        src += RenderManager::headerShaderPart();
        src += RenderManager::bufferShaderPart("srcBuffer", 0, 0, format, indexed, 1, paletteFormat);
        src +=
R"(
uniform vec2 pos;
layout(std430, binding = 0) buffer storageData
{
    Colour colour;
};

layout (local_size_x = 1, local_size_y = 1) in;
void main() {
    colour = srcBuffer(pos);
}
)";
    }break;
    default: break;
    }

    return src;
}

Colour ColourPickProgram::pick(const Buffer *const src, const Buffer *const srcPalette, const Vec2 &pos)
{
    QOpenGLShaderProgram &program = this->program();
    program.bind();

    glUniform2fv(program.uniformLocation("pos"), 1, (GLfloat *)&pos);
//    glUniform2f(program.uniformLocation("pos"), pos.x(), pos.y());

    qApp->renderManager.bindIndexedBufferShaderPart(program, "srcBuffer", 0, src, indexed, 1, srcPalette);

    Colour colour;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, storageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Colour), &colour, GL_STREAM_READ);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    void *const mapping = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Colour), GL_MAP_READ_BIT);
    memcpy(&colour, mapping, sizeof(Colour));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    // TODO: paint colour glitchy with valid index
    // problem with struct padding? No, indices are valid
//    storageData.colour.index = INDEX_INVALID;//////////////////////////////////////////
//    storageData.colour.index = 7;//////////////////////////////////////////
//    qDebug() << storageData.colour.index;/////////////////////////
    return colour;
}

} // namespace GfxPaint
