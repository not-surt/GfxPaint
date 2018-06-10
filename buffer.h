#ifndef BUFFER_H
#define BUFFER_H

#include <QImage>
#include <QSharedData>
#include <QSharedDataPointer>

#include "opengl.h"

namespace GfxPaint {

class BufferData : public QSharedData, public OpenGL {
public:
    class Format {
    public:
        enum class ComponentType {
            Invalid = -1,
            UNorm,
            SNorm,
            UInt,
            SInt,
            Float,
        };

        Format(const ComponentType type = ComponentType::Invalid, const int size = 0, const int count = 0) :
            componentType(type), componentSize(size), componentCount(count)
        {}
        inline bool operator==(const Format &rhs) const {
            return this->componentType == rhs.componentType
                    && this->componentSize == rhs.componentSize
                    && this->componentCount == rhs.componentCount;
        }
        inline bool operator!=(const Format &rhs) const { return !this->operator==(rhs); }
        inline bool operator<(const Format &rhs) const {
            return this->componentType < rhs.componentType
                    || this->componentSize < rhs.componentSize
                    || this->componentCount < rhs.componentCount;
        }

        bool isValid() const {
            return componentType != ComponentType::Invalid;
        }

        ComponentType componentType;
        int componentSize;
        int componentCount;

        struct ComponentSizeInfo {
            GLenum type;
            GLuint scale;
        };
        struct ComponentInfo {
            QString shaderSamplerType;
            QString shaderImageType;
            QList<GLenum> formats;
            QList<QString> shaderValueTypes;
            QMap<int, ComponentSizeInfo> sizes;
        };
        struct FormatInfo {
            GLint internalFormat;
            QString shaderImageFormat;
        };

        static const QMap<ComponentType, ComponentInfo> components;
        static const QMap<Format, FormatInfo> formats;

        ComponentInfo componentInfo() const {
            Q_ASSERT(components.contains(this->componentType));
            return components[this->componentType];
        }
        FormatInfo formatInfo() const {
            Q_ASSERT(formats.contains(*this));
            return formats[*this];
        }
        QString shaderSamplerType() const { return componentInfo().shaderSamplerType; }
        QString shaderImageType() const { return componentInfo().shaderImageType; }
        GLenum format() const { return componentInfo().formats[this->componentCount - 1]; }
        QString shaderValueType() const { return componentInfo().shaderValueTypes[this->componentCount - 1]; }
        QString shaderScalarValueType() const { return componentInfo().shaderValueTypes[0]; }
        GLenum type() const {
            Q_ASSERT(componentInfo().sizes.contains(this->componentSize));
            return componentInfo().sizes[this->componentSize].type;
        }
        GLuint scale() const {
            Q_ASSERT(componentInfo().sizes.contains(this->componentSize));
            return componentInfo().sizes[this->componentSize].scale;
        }
        GLint internalFormat() const { return formatInfo().internalFormat; }
        QString shaderImageFormat() const { return formatInfo().shaderImageFormat; }
    };

    BufferData();
    BufferData(const QSize size, const Format format, const GLvoid *const data = nullptr);
    explicit BufferData(const BufferData &other);
    ~BufferData();
    inline bool operator==(const BufferData &rhs) const {
        return this->size == rhs.size
                && this->format == rhs.format
                && this->texture == rhs.texture
                && this->framebuffer == rhs.framebuffer;
    }
    inline bool operator!=(const BufferData &rhs) const { return !this->operator==(rhs); }

    bool isNull() const;

    int width() const { return size.width(); }
    int height() const { return size.height(); }
    QRect rect() const { return QRect(QPoint(0, 0), size); }

    void copy(const BufferData &other, const QRect &from, const QPoint &to);
    void copy(const BufferData &other);
    void blit(const BufferData &other, const QRect &from, const QRect &to);

    void readPixel(const QPoint &pos, GLvoid *const pixel);
    void writePixel(const QPoint &pos, const GLvoid *const pixel);

    const QSize size;
    const Format format;
    const GLuint texture;
    const GLuint framebuffer;

protected:
    static GLuint createTexture(const QSize size, const Format format, const GLvoid *const data);
    static GLuint createFramebuffer(const Format format, const GLuint texture);
};

class Buffer {
public:
    using Format = BufferData::Format;

    explicit Buffer();
    explicit Buffer(const QSize size, const Format format, const GLvoid *const data = nullptr);
    Buffer(const Buffer &other);
    inline Buffer &operator=(const Buffer &rhs) { data = rhs.data; return *this; }
    inline bool operator==(const Buffer &rhs) const { return data == rhs.data; }
    inline bool operator!=(const Buffer &rhs) const { return !this->operator==(rhs); }

    bool isNull() const { return data->isNull(); }

    const QSize &size() const { return data->size; }
    int width() const { return data->size.width(); }
    int height() const { return data->size.height(); }
    QRect rect() const { return data->rect(); }
    const Format &format() const { return data->format; }
    GLuint texture() const { return data->texture; }
    GLuint framebuffer() { return data->framebuffer; }

    void copy(const Buffer &other, const QRect &from, const QPoint &to) { data->copy(*other.data, from, to); }
    void copy(const Buffer &other) { data->copy(*other.data); }
    void blit(const Buffer &other, const QRect &from, const QRect &to)  { data->blit(*other.data, from, to); }

    void readPixel(const QPoint &pos, GLvoid *const pixel) { this->data->readPixel(pos, pixel); }
    void writePixel(const QPoint &pos, const GLvoid *const pixel) { this->data->writePixel(pos, pixel); }

    void bindTextureUnit(const GLuint textureUnit) const;
    void bindImageUnit(const GLuint imageUnit);
    void bindImageUnit(const GLuint imageUnit) const;
    void bindFramebuffer(const QRect &rect, const GLenum target = GL_FRAMEBUFFER);
    void bindFramebuffer(const GLenum target = GL_FRAMEBUFFER);

    void clear();
    void clearUInt(const GLuint r = 0, const GLuint g = 0, const GLuint b = 0, const GLuint a = 0);
    void clearSInt(const GLint r = 0, const GLint g = 0, const GLint b = 0, const GLint a = 0);
    void clearFloat(const GLfloat r = 0.0, const GLfloat g = 0.0, const GLfloat b = 0.0, const GLfloat a = 0.0);

protected:
    QSharedDataPointer<BufferData> data;
};

} // namespace GfxPaint

#endif // BUFFER_H
