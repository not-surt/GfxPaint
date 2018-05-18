#include "newbufferdialog.h"
#include "ui_newbufferdialog.h"

#include <QSettings>
#include <QDebug>
#include "buffer.h"

namespace GfxPaint {

static const struct {
    QString label;
    Buffer::Buffer::Format::ComponentType componentType;
} componentTypes[] = {
{ "Unsigned Normalized", Buffer::Format::ComponentType::UNorm},
{ "Signed Normalized", Buffer::Format::ComponentType::SNorm},
{ "Unsigned Integer", Buffer::Format::ComponentType::UInt},
{ "Signed Integer", Buffer::Format::ComponentType::SInt},
{ "Floating-point", Buffer::Format::ComponentType::Float},
};

NewBufferDialog::NewBufferDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    ui(new Ui::NewBufferDialog)
{
    ui->setupUi(this);

    QObject::connect(ui->componentTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](const int index){
        const Buffer::Format::ComponentType componentType = static_cast<Buffer::Format::ComponentType>(index);
        ui->componentSizeComboBox->clear();
        for (int i = 1; i <= 4; ++i) {
            if (BufferData::Format::components[componentType].sizes.contains(i)) {
                ui->componentSizeComboBox->addItem(QString::number(i * 8) + " bpc", i);
            }
        }
    });

    ui->componentTypeComboBox->clear();
    for (auto component : componentTypes) {
        if (component.label.isEmpty()) ui->componentTypeComboBox->insertSeparator(ui->componentTypeComboBox->count());
        else ui->componentTypeComboBox->addItem(component.label, static_cast<int>(component.componentType));
    }

    ui->componentCountComboBox->clear();
    for (int i = 1; i <= 4; ++i) {
        ui->componentCountComboBox->addItem(QString::number(i) + " channel" + (i > 1 ? "s" : ""), i);
    }

    QSettings settings;
    if (settings.contains("window/newBuffer/geometry")) setGeometry(settings.value("window/newBuffer/geometry").toRect());
    if (settings.contains("window/newBuffer/imageSize")) {
        QSize imageSize = settings.value("window/newBuffer/imageSize").toSize();
        ui->imageWidthSpinBox->setValue(imageSize.width());
        ui->imageHeightSpinBox->setValue(imageSize.height());
    }
    if (settings.contains("window/newBuffer/pixelRatio")) {
        QSizeF pixelRatio = settings.value("window/newBuffer/pixelRatio").toSizeF();
        ui->pixelWidthSpinBox->setValue(pixelRatio.width());
        ui->pixelHeightSpinBox->setValue(pixelRatio.height());
    }
}

NewBufferDialog::~NewBufferDialog()
{
    delete ui;
}

QSize NewBufferDialog::imageSize() const
{
    return QSize(ui->imageWidthSpinBox->value(), ui->imageHeightSpinBox->value());
}

Buffer::Format NewBufferDialog::format() const
{
    return Buffer::Format(static_cast<Buffer::Format::ComponentType>(ui->componentTypeComboBox->currentData().toInt()), ui->componentSizeComboBox->currentData().toInt(), ui->componentCountComboBox->currentData().toInt());
}

QSizeF NewBufferDialog::pixelRatio() const
{
    return QSizeF(ui->pixelWidthSpinBox->value(), ui->pixelHeightSpinBox->value());
}

void NewBufferDialog::hideEvent(QHideEvent *event)
{
    QSettings settings;
    settings.setValue("window/newBuffer/geometry", normalGeometry());
    settings.setValue("window/newBuffer/imageSize", imageSize());
    settings.setValue("window/newBuffer/pixelRatio", pixelRatio());
    QDialog::hideEvent(event);
}

} // namespace GfxPaint
