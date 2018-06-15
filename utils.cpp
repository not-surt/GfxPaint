#include "utils.h"

#include <QtMath>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include "application.h"
#include "type.h"

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

mat4 qTransformToMat4(const QTransform &transform)
{
    return mat4{
        vec4{static_cast<GLfloat>(transform.m11()), static_cast<GLfloat>(transform.m12()), static_cast<GLfloat>(transform.m13()), 0.0},
        vec4{static_cast<GLfloat>(transform.m21()), static_cast<GLfloat>(transform.m22()), static_cast<GLfloat>(transform.m23()), 0.0},
        vec4{static_cast<GLfloat>(transform.m31()), static_cast<GLfloat>(transform.m32()), static_cast<GLfloat>(transform.m33()), 0.0},
        vec4{0.0, 0.0, 0.0, 1.0}
    };
}

QTransform qTransformFromMat4(const mat4 &matrix)
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

Buffer bufferFromImageFile(const QString &filename, Buffer *const palette, Colour *const transparent)
{
    static const QMap<QImage::Format, QImage::Format> imageFormatConversion = {
        {QImage::Format_Mono, QImage::Format_Indexed8},
        {QImage::Format_Grayscale8, QImage::Format_Indexed8},
        {QImage::Format_RGB32, QImage::Format_ARGB32},
//        {QImage::Format_RGB32, QImage::Format_RGB888},
        {QImage::Format_ARGB32_Premultiplied, QImage::Format_ARGB32},
    };
    static const QMap<QImage::Format, Buffer::Format> imageFromatToBufferFormat = {
        {QImage::Format_Indexed8, Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 1)},
        {QImage::Format_RGB888, Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 3)},
        {QImage::Format_ARGB32, Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 4)},
    };
    static const Buffer::Format paletteFormat = Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 4);

    Buffer buffer;
    QImage image(filename);

//    image = image.convertToFormat(QImage::Format_RGB888);
//    image = image.rgbSwapped();
//    image = image.convertToFormat(QImage::Format_ARGB32);

    int transparentIndex = -1;
    QColor transparentColour;
    // Hackily set transparent index if only one partially transparent index and is fully transparent
    if (image.format() == QImage::Format::Format_Indexed8) {
        for (int index = 0; index < image.colorTable().length(); ++index) {
            const int alpha = qAlpha(image.color(index));
            if (transparentIndex == -1 && alpha == 0) {
                transparentIndex = index;
            }
            else if (transparentIndex != -1 && alpha > 0 && alpha < 255) {
                transparentIndex = -1;
                break;
            }
        }
        if (transparentIndex != -1) {
            const QRgb colour = image.color(transparentIndex);
            image.setColor(transparentIndex, qRgba(qRed(colour), qGreen(colour), qBlue(colour), 255));
        }
    }
    // Hackily set transparent colour if only one partially transparent colour and is fully transparent
    else if (image.format() == QImage::Format::Format_ARGB32) {
        for (const QRgb *pixel = reinterpret_cast<const QRgb *>(image.constBits()); pixel < reinterpret_cast<const QRgb *>(image.constBits() + image.sizeInBytes()); pixel += sizeof(QRgb)) {
            const int alpha = qAlpha(*pixel);
            if (!transparentColour.isValid() && alpha == 0) {
                transparentColour = *pixel;
            }
            else if (transparentColour.isValid() && alpha > 0 && alpha < 255) {
                transparentColour = QColor();
                break;
            }
        }
        qDebug() << "isv?" << transparentColour.isValid();//////////////////////////////////
        if (transparentColour.isValid()) {
            for (QRgb *pixel = reinterpret_cast<QRgb *>(image.bits()); pixel < reinterpret_cast<const QRgb *>(image.constBits() + image.sizeInBytes()); pixel += sizeof(QRgb)) {
                const int alpha = qAlpha(*pixel);
                if (alpha < 255) *pixel = transparentColour.rgba();
            }
            image = image.convertToFormat(QImage::Format_RGB32, Qt::AutoColor | Qt::ThresholdDither | Qt::ThresholdAlphaDither | Qt::AvoidDither);
            qDebug() << "CONVERTED!" << image.format();//////////////////////////////////
        }
    }
    if (transparent) *transparent = Colour{!transparentColour.isValid() ? RGBA_INVALID : qColorToVec4(transparentColour), transparentIndex == -1 ? INDEX_INVALID : transparentIndex};

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

float stairstep(const float value, const float size) {
    return round(value / size) * size;
}

float snap(const float offset, const float size, const float target, const bool relative, const float relativeTo) {
    const float shift = (relative ? relativeTo : offset);
    return size != 0.0 ? stairstep(target - shift, size) + shift : target;
}

QPointF snap2d(const QPointF offset, const QSizeF size, const QPointF target, const bool relative, const QPointF relativeTo) {
    return QPointF(snap(offset.x(), size.width(), target.x(), relative, relativeTo.x()),
                   snap(offset.y(), size.height(), target.y(), relative, relativeTo.y()));
}

void rotateScaleAtOrigin(QTransform &transform, const qreal rotation, const qreal scaling, const QPointF origin)
{
    const QPointF pointBefore = (transform.inverted()).map(origin);
    transform *= QTransform().scale(scaling, scaling) * QTransform().rotate(rotation);
    const QPointF pointAfter = (transform.inverted()).map(origin);
    const QPointF offset = pointAfter - pointBefore;
    transform = QTransform().translate(offset.x(), offset.y()) * transform;
}

QTransform transformPointToPoint(const QPointF origin, const QPointF from, const QPointF to)
{
    const QVector2D fromVector = QVector2D(from - origin);
    const QVector2D toVector = QVector2D(to - origin);
    const qreal scaling = toVector.length() / fromVector.length();
    const qreal rotation = qRadiansToDegrees(atan2(toVector.y(), toVector.x()) - atan2(fromVector.y(), fromVector.x()));
    QTransform transform;
    rotateScaleAtOrigin(transform, rotation, scaling, origin);
    return transform;
}

bool KeyEventTransition::eventTest(QEvent *event) {
    const QStateMachine::WrappedEvent *const wrappedEvent = static_cast<const QStateMachine::WrappedEvent *>(event);
    const QKeyEvent *const keyEvent = static_cast<const QKeyEvent *>(wrappedEvent->event());
    return QEventTransition::eventTest(event) &&
            keyEvent->key() == key() &&
            (modifierMask() == Qt::NoModifier || keyEvent->modifiers() & modifierMask()) &&
            !keyEvent->isAutoRepeat();
}

bool MouseEventTransition::eventTest(QEvent *event) {
    const QStateMachine::WrappedEvent *const wrappedEvent = static_cast<const QStateMachine::WrappedEvent *>(event);
    const QMouseEvent *const mouseEvent = static_cast<const QMouseEvent *>(wrappedEvent->event());
    return QEventTransition::eventTest(event) &&
            mouseEvent->button() == button() &&
            (modifierMask() == Qt::NoModifier || mouseEvent->modifiers() & modifierMask());
}

bool ModifierKeyEventTransition::eventTest(QEvent *event) {
    static const QSet<Qt::Key> modifiers = {Qt::Key_Shift, Qt::Key_Control, Qt::Key_Meta, Qt::Key_Alt, Qt::Key_AltGr};
    const QStateMachine::WrappedEvent *const wrappedEvent = static_cast<const QStateMachine::WrappedEvent *>(event);
    if (wrappedEvent->object() == eventSource()) {
        if ((wrappedEvent->event()->type() == QEvent::KeyPress || wrappedEvent->event()->type() == QEvent::KeyRelease)) {
            const QKeyEvent *const keyEvent = static_cast<const QKeyEvent *>(wrappedEvent->event());
            if (modifiers.contains(static_cast<Qt::Key>(keyEvent->key()))) return true;
        }
    }
    return false;
}

} // namespace GfxPaint
