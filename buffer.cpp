#include "buffer.h"

#include <limits>

namespace GfxPaint {

const std::map<BufferData::Format::ComponentType, std::string> BufferData::Format::componentTypeNames = {
    {BufferData::Format::ComponentType::Invalid, "Invalid"},
    {BufferData::Format::ComponentType::UNorm, "Unsigned Normalised"},
    {BufferData::Format::ComponentType::SNorm, "Signed Normalised"},
    {BufferData::Format::ComponentType::UInt, "Unsigned Integer"},
    {BufferData::Format::ComponentType::SInt, "Signed Integer"},
    {BufferData::Format::ComponentType::Float, "Floating-point"},
};

const std::map<BufferData::Format::ComponentType, BufferData::Format::ComponentInfo> BufferData::Format::components = {
    {ComponentType::UNorm, BufferData::Format::ComponentInfo{"sampler2D", "image2D", {GL_RED, GL_RG, GL_RGB, GL_RGBA}, {"float", "vec2", "vec3", "vec4"}, std::map<int, ComponentSizeInfo>{
         {1, {GL_UNSIGNED_BYTE, 1}},
         {2, {GL_UNSIGNED_SHORT, 1}},
     }}},
    {ComponentType::SNorm, BufferData::Format::ComponentInfo{"sampler2D", "image2D", {GL_RED, GL_RG, GL_RGB, GL_RGBA}, {"float", "vec2", "vec3", "vec4"}, std::map<int, ComponentSizeInfo>{
         {1, {GL_BYTE, 1}},
         {2, {GL_SHORT, 1}},
     }}},
    {ComponentType::UInt, BufferData::Format::ComponentInfo{"usampler2D", "uimage2D", {GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER}, {"uint", "uvec2", "uvec3", "uvec4"}, std::map<int, ComponentSizeInfo>{
         {1, {GL_UNSIGNED_BYTE, static_cast<GLuint>(std::numeric_limits<GLubyte>::max())}},
         {2, {GL_UNSIGNED_SHORT, static_cast<GLuint>(std::numeric_limits<GLushort>::max())}},
         {4, {GL_UNSIGNED_INT, static_cast<GLuint>(std::numeric_limits<GLuint>::max())}},
     }}},
    {ComponentType::SInt, BufferData::Format::ComponentInfo{"isampler2D", "iimage2D", {GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER}, {"int", "ivec2", "ivec3", "ivec4"}, std::map<int, ComponentSizeInfo>{
         {1, {GL_BYTE, static_cast<GLuint>(std::numeric_limits<GLbyte>::max())}},
         {2, {GL_SHORT, static_cast<GLuint>(std::numeric_limits<GLshort>::max())}},
         {4, {GL_INT, static_cast<GLuint>(std::numeric_limits<GLint>::max())}},
     }}},
    {ComponentType::Float, BufferData::Format::ComponentInfo{"sampler2D", "image2D", {GL_RED, GL_RG, GL_RGB, GL_RGBA}, {"float", "vec2", "vec3", "vec4"}, std::map<int, ComponentSizeInfo>{
         {2, {GL_HALF_FLOAT, 1}},
         {4, {GL_FLOAT, 1}},
     }}},
};

const std::map<BufferData::Format, BufferData::Format::FormatInfo> BufferData::Format::formats = {
    {Format(ComponentType::UNorm, 1, 1), {GL_R8, "r8"}},
    {Format(ComponentType::UNorm, 1, 2), {GL_RG8, "rg8"}},
    {Format(ComponentType::UNorm, 1, 3), {GL_RGB8, "rgb8"}},
    {Format(ComponentType::UNorm, 1, 4), {GL_RGBA8, "rgba8"}},
    {Format(ComponentType::UNorm, 2, 1), {GL_R16, "r16"}},
    {Format(ComponentType::UNorm, 2, 2), {GL_RG16, "rg16"}},
    {Format(ComponentType::UNorm, 2, 3), {GL_RGB16, "rgb16"}},
    {Format(ComponentType::UNorm, 2, 4), {GL_RGBA16, "rgba16"}},

    {Format(ComponentType::SNorm, 1, 1), {GL_R8_SNORM, "r8_snorm"}},
    {Format(ComponentType::SNorm, 1, 2), {GL_RG8_SNORM, "rg8_snorm"}},
    {Format(ComponentType::SNorm, 1, 3), {GL_RGB8_SNORM, "rgb8_snorm"}},
    {Format(ComponentType::SNorm, 1, 4), {GL_RGBA8_SNORM, "rgba8_snorm"}},
    {Format(ComponentType::SNorm, 2, 1), {GL_R16_SNORM, "r16_snorm"}},
    {Format(ComponentType::SNorm, 2, 2), {GL_RG16_SNORM, "rg16_snorm"}},
    {Format(ComponentType::SNorm, 2, 3), {GL_RGB16_SNORM, "rgb16_snorm"}},
    {Format(ComponentType::SNorm, 2, 4), {GL_RGBA16_SNORM, "rgba16_snorm"}},

    {Format(ComponentType::UInt, 1, 1), {GL_R8UI, "r8ui"}},
    {Format(ComponentType::UInt, 1, 2), {GL_RG8UI, "rg8ui"}},
    {Format(ComponentType::UInt, 1, 3), {GL_RGB8UI, "rgb8ui"}},
    {Format(ComponentType::UInt, 1, 4), {GL_RGBA8UI, "rgba8ui"}},
    {Format(ComponentType::UInt, 2, 1), {GL_R16UI, "r16ui"}},
    {Format(ComponentType::UInt, 2, 2), {GL_RG16UI, "rg16ui"}},
    {Format(ComponentType::UInt, 2, 3), {GL_RGB16UI, "rgb16ui"}},
    {Format(ComponentType::UInt, 2, 4), {GL_RGBA16UI, "rgba16ui"}},
    {Format(ComponentType::UInt, 4, 1), {GL_R32UI, "r32ui"}},
    {Format(ComponentType::UInt, 4, 2), {GL_RG32UI, "rg32ui"}},
    {Format(ComponentType::UInt, 4, 3), {GL_RGB32UI, "rgb32ui"}},
    {Format(ComponentType::UInt, 4, 4), {GL_RGBA32UI, "rgba32ui"}},

    {Format(ComponentType::SInt, 1, 1), {GL_R8I, "r8i"}},
    {Format(ComponentType::SInt, 1, 2), {GL_RG8I, "rg8i"}},
    {Format(ComponentType::SInt, 1, 3), {GL_RGB8I, "rgb8i"}},
    {Format(ComponentType::SInt, 1, 4), {GL_RGBA8I, "rgba8i"}},
    {Format(ComponentType::SInt, 2, 1), {GL_R16I, "r16i"}},
    {Format(ComponentType::SInt, 2, 2), {GL_RG16I, "rg16i"}},
    {Format(ComponentType::SInt, 2, 3), {GL_RGB16I, "rgb16i"}},
    {Format(ComponentType::SInt, 2, 4), {GL_RGBA16I, "rgba16i"}},
    {Format(ComponentType::SInt, 4, 1), {GL_R32I, "r32i"}},
    {Format(ComponentType::SInt, 4, 2), {GL_RG32I, "rg32i"}},
    {Format(ComponentType::SInt, 4, 3), {GL_RGB32I, "rgb32i"}},
    {Format(ComponentType::SInt, 4, 4), {GL_RGBA32I, "rgba32i"}},

    {Format(ComponentType::Float, 2, 1), {GL_R16F, "r16f"}},
    {Format(ComponentType::Float, 2, 2), {GL_RG16F, "rg16f"}},
    {Format(ComponentType::Float, 2, 3), {GL_RGB16F, "rgb16f"}},
    {Format(ComponentType::Float, 2, 4), {GL_RGBA16F, "rgba16f"}},
    {Format(ComponentType::Float, 4, 1), {GL_R32F, "r32f"}},
    {Format(ComponentType::Float, 4, 2), {GL_RG32F, "rg32f"}},
    {Format(ComponentType::Float, 4, 3), {GL_RGB32F, "rgb32f"}},
    {Format(ComponentType::Float, 4, 4), {GL_RGBA32F, "rgba32f"}},
};

BufferData::BufferData() :
    QSharedData(), OpenGL(false),
    size(0, 0), format(),
    texture(0),
    framebuffer(0)
{
}

BufferData::BufferData(const QSize size, const Format format, const GLvoid *const data) :
    QSharedData(), OpenGL(true),
    size(size), format(format),
    texture(createTexture(size, format, data)),
    framebuffer(createFramebuffer(format, texture))
{
    Q_ASSERT(Format::formats.contains(format));
}

BufferData::BufferData(const BufferData &other) :
    QSharedData(other), OpenGL(!other.isNull()),
    size(other.size), format(other.format),
    texture(createTexture(size, format, nullptr)),
    framebuffer(createFramebuffer(format, texture))
{
    Q_ASSERT(Format::formats.contains(format));
    copy(other);
}

BufferData::~BufferData()
{
    if (!isNull()) {
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &texture);        
    }
}

bool BufferData::isNull() const
{
    return size.isNull();
}

void BufferData::copy(const BufferData &other, const QRect &from, const QPoint &to)
{
    Q_ASSERT(format == other.format);

    glCopyImageSubData(other.texture, GL_TEXTURE_2D, 0, from.x(), from.y(), 0,
                       texture, GL_TEXTURE_2D, 0, to.x(), to.y(), 0,
                       from.width(), from.height(), 1);
}

void BufferData::copy(const BufferData &other)
{
    Q_ASSERT(size == other.size);

    copy(other, rect(), {0, 0});
}

void BufferData::blit(const BufferData &other, const QRect &from, const QRect &to)
{
    Q_ASSERT(format == other.format);

    FramebufferBinder readBinder(GL_READ_FRAMEBUFFER, other.framebuffer);
    FramebufferBinder drawBinder(GL_DRAW_FRAMEBUFFER, framebuffer);
    glBlitFramebuffer(from.x(), from.y(), from.width(), from.height(), to.x(), to.y(), to.width(), to.height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void BufferData::readPixel(const QPoint &pos, GLvoid *const pixel)
{
    FramebufferBinder readBinder(GL_READ_FRAMEBUFFER, framebuffer);
    //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glReadPixels(pos.x(), pos.y(), 1, 1, format.format(), format.type(), pixel);
}

void BufferData::writePixel(const QPoint &pos, const GLvoid *const pixel)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x(), pos.y(), 1, 1, format.format(), format.type(), pixel);
}

GLuint BufferData::createTexture(const QSize size, const Format format, const GLvoid *const data)
{
    OpenGLFunctions gl;
    gl.initializeOpenGLFunctions();

    GLuint texture;
    gl.glGenTextures(1, &texture);
    TextureBinder textureBinder(GL_TEXTURE_2D, texture);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl.glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat(), size.width(), size.height(), 0, format.format(), format.type(), data);
    return texture;
}

GLuint BufferData::createFramebuffer(const Format format, const GLuint texture)
{
    OpenGLFunctions gl;
    gl.initializeOpenGLFunctions();

    GLuint framebuffer;
    gl.glGenFramebuffers(1, &framebuffer);
    FramebufferBinder framebufferBinder(GL_FRAMEBUFFER, framebuffer);
    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    gl.glReadBuffer(GL_COLOR_ATTACHMENT0);
    GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    gl.glDrawBuffers(1, &drawBuffer);
    Q_ASSERT(gl.glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    return framebuffer;
}

Buffer::Buffer() :
    data(new BufferData())
{
}

Buffer::Buffer(const QSize size, const Format format, const GLvoid *const data) :
    data(new BufferData(size, format, data))
{
}

Buffer::Buffer(const Buffer &other) :
    data(other.data)
{
}

void Buffer::bindTextureUnit(const GLuint textureUnit) const
{
    BufferData *const data = const_cast<BufferData *>(this->data.constData());
    data->glActiveTexture(GL_TEXTURE0 + textureUnit);
    data->glBindTexture(GL_TEXTURE_2D, data->texture);
}

void Buffer::bindImageUnit(const GLuint imageUnit) const
{
    BufferData *const data = const_cast<BufferData *>(this->data.constData());
    data->glBindImageTexture(imageUnit, data->texture, 0, GL_FALSE, 0, GL_READ_WRITE, static_cast<GLenum>(data->format.internalFormat()));
}

void Buffer::bindFramebuffer(const QRect &rect, const GLenum target)
{
    data->glBindFramebuffer(target, data->framebuffer);
    data->glViewport(rect.x(), rect.y(), rect.width(), rect.height());
    data->glEnable(GL_SCISSOR_TEST);
    data->glScissor(rect.x(), rect.y(), rect.width(), rect.height());
}

void Buffer::bindFramebuffer(const GLenum target)
{
    bindFramebuffer(rect(), target);
}

void Buffer::clear()
{
    switch (data->format.componentType) {
    case Format::ComponentType::UInt: {
        clearUInt();
    } break;
    case Format::ComponentType::SInt: {
        clearSInt();
    } break;
    default: {
        clearFloat();
    } break;
    }
}

void Buffer::clearUInt(const GLuint r, const GLuint g, const GLuint b, const GLuint a)
{
    FramebufferBinder framebufferBinder(GL_FRAMEBUFFER, data->framebuffer, rect());
    const GLuint values[] = {r, g, b, a};
    data->glClearBufferuiv(GL_COLOR, 0, values);
}

void Buffer::clearSInt(const GLint r, const GLint g, const GLint b, const GLint a)
{
    FramebufferBinder framebufferBinder(GL_FRAMEBUFFER, data->framebuffer, rect());
    const GLint values[] = {r, g, b, a};
    data->glClearBufferiv(GL_COLOR, 0, values);
}

void Buffer::clearFloat(const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat a)
{
    FramebufferBinder framebufferBinder(GL_FRAMEBUFFER, data->framebuffer, rect());
    const GLfloat values[] = {r, g, b, a};
    data->glClearBufferfv(GL_COLOR, 0, values);
}

} // namespace GfxPaint
