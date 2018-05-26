#include "dabeditorwidget.h"
#include "ui_dabeditorwidget.h"

#include <QVector2D>
#include <QtMath>

#include "rendermanager.h"

namespace GfxPaint {

DabEditorWidget::DabEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DabEditorWidget),
    dab()
{
    ui->setupUi(this);

    QDoubleValidator *const validator = new QDoubleValidator;
    validator->setDecimals(3);
    ui->ratioLineEdit->setValidator(validator);

    QObject::connect(ui->spaceComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->typeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->metricComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->widthSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->heightSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->fixedRatioCheckBox, &QCheckBox::toggled, this, &DabEditorWidget::updateDab);
    QObject::connect(ui->ratioLineEdit, &QLineEdit::textChanged, this, &DabEditorWidget::updateRatio);
    QObject::connect(ui->angleSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->originXSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->originYSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->hardnessSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->opacitySpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->blendModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &DabEditorWidget::updateDab);
    QObject::connect(ui->composeModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &DabEditorWidget::updateDab);

    for (auto blendMode : RenderManager::blendModes) {
        ui->blendModeComboBox->addItem(blendMode.label);
    }

    for (auto composeMode : RenderManager::composeModes) {
        ui->composeModeComboBox->addItem(composeMode.label);
    }
}

DabEditorWidget::~DabEditorWidget()
{
    delete ui;
}

void DabEditorWidget::setDab(const Dab &dab)
{
    if (this->dab != dab) {
        this->dab = dab;
        updateWidgets();
    }
}

void DabEditorWidget::updateWidgets()
{
    const QWidgetList widgets = {
        ui->spaceComboBox, ui->typeComboBox, ui->metricComboBox,
        ui->widthSpinBox, ui->heightSpinBox, ui->fixedRatioCheckBox, ui->ratioLineEdit,
        ui->angleSpinBox,
        ui->originXSpinBox, ui->originYSpinBox,
        ui->hardnessSpinBox, ui->opacitySpinBox,
        ui->blendModeComboBox, ui->composeModeComboBox,
    };
    for (auto widget : widgets) widget->blockSignals(true);
    ui->spaceComboBox->setCurrentIndex(static_cast<int>(dab.space));
    ui->typeComboBox->setCurrentIndex(static_cast<int>(dab.type));
    ui->metricComboBox->setCurrentIndex(dab.metric);
    ui->widthSpinBox->setValue(dab.size.width());
    ui->heightSpinBox->setValue(dab.size.height());
    ui->fixedRatioCheckBox->setChecked(dab.fixedRatio);
    ui->angleSpinBox->setValue(dab.angle);
    ui->originXSpinBox->setValue(dab.origin.x());
    ui->originYSpinBox->setValue(dab.origin.y());
    ui->hardnessSpinBox->setValue(dab.hardness);
    ui->opacitySpinBox->setValue(dab.opacity);
    ui->blendModeComboBox->setCurrentIndex(dab.blendMode);
    ui->composeModeComboBox->setCurrentIndex(dab.composeMode);
    for (auto widget : widgets) widget->blockSignals(false);
    updateDab();
}

void DabEditorWidget::updateDab()
{
    dab.space = static_cast<Space>(ui->spaceComboBox->currentIndex());
    dab.type = static_cast<Dab::Type>(ui->typeComboBox->currentIndex());
    dab.metric = ui->metricComboBox->currentIndex();

    if (ui->fixedRatioCheckBox->isChecked()) {
        if (QObject::sender() == ui->widthSpinBox) {
            ui->heightSpinBox->blockSignals(true);
            ui->heightSpinBox->setValue(ui->widthSpinBox->value() * (1.0 / dab.ratio));
            ui->heightSpinBox->blockSignals(false);
        }
        else if (QObject::sender() == ui->heightSpinBox) {
            ui->widthSpinBox->blockSignals(true);
            ui->widthSpinBox->setValue(ui->heightSpinBox->value() * dab.ratio);
            ui->widthSpinBox->blockSignals(false);
        }
    }
    else {
        dab.ratio = ui->widthSpinBox->value() / ui->heightSpinBox->value();
        ui->ratioLineEdit->blockSignals(true);
        ui->ratioLineEdit->setText(QString::number(dab.ratio, 'f', 3));
        ui->ratioLineEdit->blockSignals(false);
    }
    dab.size = {ui->widthSpinBox->value(), ui->heightSpinBox->value()};
    dab.fixedRatio = ui->fixedRatioCheckBox->isChecked();

    dab.angle = ui->angleSpinBox->value();
    dab.origin = {ui->originXSpinBox->value(), ui->originYSpinBox->value()};

    dab.hardness = ui->hardnessSpinBox->value();
    dab.opacity = ui->opacitySpinBox->value();

    dab.blendMode = ui->blendModeComboBox->currentIndex();
    dab.composeMode = ui->composeModeComboBox->currentIndex();

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
            ui->heightSpinBox->setValue(ui->widthSpinBox->value() * (1.0 / dab.ratio));
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
