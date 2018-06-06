#include "utils.h"

#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include "application.h"

namespace GfxPaint {

QString buildFilterString(QList<QByteArray> formats, QString allLabel)
{
    QStringList filters;
    QStringList allFilter;
    for (auto format : formats) {
        const QString filterString = "*." + format;
        filters.append(format.toUpper() + " files (" + filterString + ")");
        allFilter.append(filterString);
    }
    filters.prepend(QString("All %1 files (").arg(allLabel) + allFilter.join(" ") + ")");
    return filters.join(";;");
}

QWidget *centringWidget(QWidget *const widget)
{
    QHBoxLayout *const centringLayout = new QHBoxLayout();
    centringLayout->setMargin(0);
    centringLayout->addWidget(widget);
    QWidget *const centringWidget = new QWidget();
    centringWidget->setLayout(centringLayout);
    return centringWidget;
}

QString fileToString(const QString &path)
{
    QFile file(path);
    Q_ASSERT(file.exists());
    if (file.open(QIODevice::ReadOnly)) return QTextStream(&file).readAll();
    else return QString();
}

QTransform viewportTransform(const QSize size)
{
    return QTransform(
        2.0 / size.width(), 0.0, 0.0,
        0.0, 2.0 / size.height(), 0.0,
        -1.0, -1.0, 1.0
    );
}

mat3 qTransformToMat3(const QTransform &transform)
{
    return mat3{
        vec3{static_cast<GLfloat>(transform.m11()), static_cast<GLfloat>(transform.m12()), static_cast<GLfloat>(transform.m13())},
        vec3{static_cast<GLfloat>(transform.m21()), static_cast<GLfloat>(transform.m22()), static_cast<GLfloat>(transform.m23())},
        vec3{static_cast<GLfloat>(transform.m31()), static_cast<GLfloat>(transform.m32()), static_cast<GLfloat>(transform.m33())}
    };
}

QTransform qTransformFromMat3(const mat3 &matrix)
{
    return QTransform(
        static_cast<qreal>(matrix[0][0]), static_cast<qreal>(matrix[0][1]), static_cast<qreal>(matrix[0][2]),
        static_cast<qreal>(matrix[1][0]), static_cast<qreal>(matrix[1][1]), static_cast<qreal>(matrix[1][2]),
        static_cast<qreal>(matrix[2][0]), static_cast<qreal>(matrix[2][1]), static_cast<qreal>(matrix[2][2])
    );
}

vec4 qColorToVec4(const QColor &qColor)
{
    return vec4{static_cast<GLfloat>(qColor.redF()), static_cast<GLfloat>(qColor.greenF()), static_cast<GLfloat>(qColor.blueF()), static_cast<GLfloat>(qColor.alphaF())};
}

QColor qColorFromVec4(const vec4 &colour)
{
    return QColor(static_cast<qreal>(colour[0]), static_cast<qreal>(colour[1]), static_cast<qreal>(colour[2]), static_cast<qreal>(colour[3]));
}

Buffer bufferFromImageFile(const QString &filename, Buffer *const palette)
{
    static const QMap<QImage::Format, QImage::Format> imageFormatConversion = {
        {QImage::Format_Mono, QImage::Format_Indexed8},
        {QImage::Format_Grayscale8, QImage::Format_Indexed8},
        {QImage::Format_RGB32, QImage::Format_ARGB32},
        {QImage::Format_ARGB32_Premultiplied, QImage::Format_ARGB32},
    };
    static const QMap<QImage::Format, Buffer::Format> imageFromatToBufferFormat = {
        {QImage::Format_Indexed8, Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 1)},
        {QImage::Format_ARGB32, Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 4)},
    };
    static const Buffer::Format paletteFormat = Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 4);

    Buffer buffer;
    QImage image(filename);
    image = image.rgbSwapped();
    if (imageFormatConversion.contains(image.format())) image = image.convertToFormat(imageFormatConversion[image.format()]);
    if (imageFromatToBufferFormat.contains(image.format())) {
        buffer = Buffer(image.size(), imageFromatToBufferFormat[image.format()], image.constBits());
        if (palette && image.format() == QImage::Format::Format_Indexed8) {
            *palette = Buffer(QSize(image.colorTable().length(), 1), paletteFormat, image.colorTable().constData());
        }
    }
    return buffer;
}

void stringMultiReplace(QString &string, const QMap<QString, QString> &replacements)
{
    for (auto key : replacements.keys()) string.replace(key, replacements[key]);
}

} // namespace GfxPaint
