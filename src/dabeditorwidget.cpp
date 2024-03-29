#include "dabeditorwidget.h"
#include "ui_dabeditorwidget.h"

#include "types.h"
#include "rendermanager.h"

namespace GfxPaint {

DabEditorWidget::DabEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DabEditorWidget),
    dab()
{
    ui->setupUi(this);

    QDoubleValidator *const validator = new QDoubleValidator(this);
    validator->setDecimals(3);
    ui->ratioLineEdit->setValidator(validator);

    QObject::connect(ui->typeComboBox, qOverload<int>(&QComboBox::activated), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->metricComboBox, qOverload<int>(&QComboBox::activated), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->widthSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->heightSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->fixedRatioCheckBox, &QCheckBox::toggled, this, &DabEditorWidget::updateDab);
    QObject::connect(ui->ratioLineEdit, &QLineEdit::textChanged, this, &DabEditorWidget::updateRatio);
    QObject::connect(ui->angleSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->originXSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->originYSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->pixelSnapXComboBox, qOverload<int>(&QComboBox::activated), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->pixelSnapYComboBox, qOverload<int>(&QComboBox::activated), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->hardnessSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->opacitySpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);

    updateWidgets();
}

DabEditorWidget::~DabEditorWidget()
{
    delete ui;
}

void DabEditorWidget::setDab(const Brush::Dab &dab)
{
    if (this->dab != dab) {
        this->dab = dab;
        updateWidgets();
    }
}

void DabEditorWidget::updateWidgets()
{
    const QWidgetList widgets = {
        ui->typeComboBox, ui->metricComboBox,
        ui->widthSpinBox, ui->heightSpinBox, ui->fixedRatioCheckBox, ui->ratioLineEdit,
        ui->angleSpinBox,
        ui->originXSpinBox, ui->originYSpinBox,
        ui->pixelSnapXComboBox, ui->pixelSnapYComboBox,
        ui->hardnessSpinBox, ui->opacitySpinBox,
    };
    for (auto widget : widgets) widget->blockSignals(true);
    ui->typeComboBox->setCurrentIndex(static_cast<int>(dab.type));
    ui->metricComboBox->setCurrentIndex(dab.metric);
    ui->widthSpinBox->setValue(dab.size.x());
    ui->heightSpinBox->setValue(dab.size.y());
    ui->fixedRatioCheckBox->setChecked(dab.fixedRatio);
    ui->angleSpinBox->setValue(dab.angle);
    ui->originXSpinBox->setValue(dab.origin.x());
    ui->originYSpinBox->setValue(dab.origin.y());
    ui->pixelSnapXComboBox->setCurrentIndex(static_cast<int>(dab.pixelSnapX));
    ui->pixelSnapYComboBox->setCurrentIndex(static_cast<int>(dab.pixelSnapY));
    ui->hardnessSpinBox->setValue(dab.hardness);
    ui->opacitySpinBox->setValue(dab.opacity);
    for (auto widget : widgets) widget->blockSignals(false);
    updateDab();
}

void DabEditorWidget::updateDab()
{
    dab.type = static_cast<Brush::Dab::Type>(ui->typeComboBox->currentIndex());
    dab.metric = ui->metricComboBox->currentIndex();

    if (ui->fixedRatioCheckBox->isChecked()) {
        if (QObject::sender() == ui->widthSpinBox) {
            ui->heightSpinBox->blockSignals(true);
            ui->heightSpinBox->setValue(ui->widthSpinBox->value() * (1.0f / dab.ratio));
            ui->heightSpinBox->blockSignals(false);
        }
        else if (QObject::sender() == ui->heightSpinBox) {
            ui->widthSpinBox->blockSignals(true);
            ui->widthSpinBox->setValue(ui->heightSpinBox->value() * dab.ratio);
            ui->widthSpinBox->blockSignals(false);
        }
    }
    else {
        dab.ratio = static_cast<float>(ui->widthSpinBox->value() / ui->heightSpinBox->value());
        ui->ratioLineEdit->blockSignals(true);
        ui->ratioLineEdit->setText(QString::number(dab.ratio, 'f', 3));
        ui->ratioLineEdit->blockSignals(false);
    }
    dab.size = {static_cast<float>(ui->widthSpinBox->value()), static_cast<float>(ui->heightSpinBox->value())};
    dab.fixedRatio = ui->fixedRatioCheckBox->isChecked();

    dab.angle = ui->angleSpinBox->value();
    dab.origin = {static_cast<float>(ui->originXSpinBox->value()), static_cast<float>(ui->originYSpinBox->value())};
    dab.pixelSnapX = static_cast<PixelSnap>(ui->pixelSnapXComboBox->currentIndex());
    dab.pixelSnapY = static_cast<PixelSnap>(ui->pixelSnapYComboBox->currentIndex());

    dab.hardness = ui->hardnessSpinBox->value();
    dab.opacity = ui->opacitySpinBox->value();

    emit dabChanged(dab);
}

void DabEditorWidget::updateRatio(const QString &string)
{
    bool isValid;
    const qreal ratio = string.toDouble(&isValid);
    if (isValid) {
        dab.ratio = ratio;
        if (ui->widthSpinBox->value() >= ui->heightSpinBox->value()) {
            ui->heightSpinBox->blockSignals(true);
            ui->heightSpinBox->setValue(ui->widthSpinBox->value() * (1.0f / dab.ratio));
            ui->heightSpinBox->blockSignals(false);
        }
        else {
            ui->widthSpinBox->blockSignals(true);
            ui->widthSpinBox->setValue(ui->heightSpinBox->value() * dab.ratio);
            ui->widthSpinBox->blockSignals(false);
        }
        updateDab();
    }
}

} // namespace GfxPaint
