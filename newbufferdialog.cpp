#include "newbufferdialog.h"
#include "ui_newbufferdialog.h"

#include <QSettings>
#include "buffer.h"
#include "rendermanager.h"

namespace GfxPaint {

NewBufferDialog::NewBufferDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog(parent, flags),
    ui(new Ui::NewBufferDialog)
{
    ui->setupUi(this);

    QObject::connect(ui->componentTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](const int index){
        const Buffer::Format::ComponentType componentType = static_cast<Buffer::Format::ComponentType>(index);
        ui->componentSizeComboBox->clear();
        for (int i = 1; i <= 4; ++i) {
            if (BufferData::Format::components[componentType].sizes.contains(i)) {
                ui->componentSizeComboBox->addItem(QString::number(i * 8) + " bpc", i);
            }
        }
    });

    for (auto key : BufferData::Format::componentTypeNames.keys()) {
        if (key != BufferData::Format::ComponentType::Invalid) ui->componentTypeComboBox->addItem(BufferData::Format::componentTypeNames[key], static_cast<int>(key));
    }

    for (int i = 1; i <= 4; ++i) {
        ui->componentCountComboBox->addItem(QString::number(i) + " channel" + (i > 1 ? "s" : ""), i);
    }

    for (auto blendMode : RenderManager::blendModes) {
        ui->blendModeComboBox->addItem(blendMode.label);
    }

    for (auto composeMode : RenderManager::composeModes) {
        ui->composeModeComboBox->addItem(composeMode.label);
    }
    ui->composeModeComboBox->setCurrentIndex(RenderManager::composeModeDefault);

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
    if (settings.contains("window/newBuffer/formatComponentType")) ui->componentTypeComboBox->setCurrentIndex(settings.value("window/newBuffer/formatComponentType").toInt());
    if (settings.contains("window/newBuffer/formatComponentSize")) ui->componentSizeComboBox->setCurrentIndex(settings.value("window/newBuffer/formatComponentSize").toInt());
    if (settings.contains("window/newBuffer/formatComponentCount")) ui->componentCountComboBox->setCurrentIndex(settings.value("window/newBuffer/formatComponentCount").toInt());
    if (settings.contains("window/newBuffer/indexed")) ui->indexedCheckBox->setChecked(settings.value("window/newBuffer/indexed").toBool());
    if (settings.contains("window/newBuffer/blendMode")) ui->blendModeComboBox->setCurrentIndex(settings.value("window/newBuffer/blendMode").toInt());
    if (settings.contains("window/newBuffer/composeMode")) ui->blendModeComboBox->setCurrentIndex(settings.value("window/newBuffer/composeMode").toInt());
    if (settings.contains("window/newBuffer/opacity")) ui->opacitySpinBox->setValue(settings.value("window/newBuffer/opacity").toFloat());
}

NewBufferDialog::~NewBufferDialog()
{
    delete ui;
}

void NewBufferDialog::hideEvent(QHideEvent *event)
{
    QSettings settings;
    settings.setValue("window/newBuffer/geometry", normalGeometry());
    settings.setValue("window/newBuffer/imageSize", imageSize());
    settings.setValue("window/newBuffer/pixelRatio", pixelRatio());
    settings.setValue("window/newBuffer/formatComponentType", ui->componentTypeComboBox->currentIndex());
    settings.setValue("window/newBuffer/formatComponentSize", ui->componentSizeComboBox->currentIndex());
    settings.setValue("window/newBuffer/formatComponentCount", ui->componentCountComboBox->currentIndex());
    settings.setValue("window/newBuffer/indexed", indexed());
    settings.setValue("window/newBuffer/blendMode", ui->blendModeComboBox->currentIndex());
    settings.setValue("window/newBuffer/composeMode", ui->composeModeComboBox->currentIndex());
    settings.setValue("window/newBuffer/opacity", ui->opacitySpinBox->value());
    QDialog::hideEvent(event);
}

} // namespace GfxPaint
