#include "strokeeditorwidget.h"
#include "ui_strokeeditorwidget.h"

namespace GfxPaint {

StrokeEditorWidget::StrokeEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StrokeEditorWidget),
    stroke()
{
    ui->setupUi(this);

    QObject::connect(ui->spaceComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->continuousCheckBox, &QCheckBox::toggled, this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->absoluteSpacingXSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->absoluteSpacingYSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->proportionalSpacingXSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->proportionalSpacingYSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->dabCountSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
}

StrokeEditorWidget::~StrokeEditorWidget()
{
    delete ui;
}

void StrokeEditorWidget::setStroke(const Stroke &stroke)
{
    if (this->stroke != stroke) {
        this->stroke = stroke;
        updateWidgets();
    }
}

void StrokeEditorWidget::updateWidgets()
{
    const QWidgetList widgets = {
        ui->spaceComboBox,
        ui->continuousCheckBox,
        ui->absoluteSpacingXSpinBox, ui->absoluteSpacingYSpinBox,
        ui->proportionalSpacingXSpinBox, ui->proportionalSpacingYSpinBox,
        ui->dabCountSpinBox,
    };
    for (auto widget : widgets) widget->blockSignals(true);
    ui->spaceComboBox->setCurrentIndex(static_cast<int>(stroke.space));
    ui->continuousCheckBox->setChecked(stroke.continuous);
    ui->absoluteSpacingXSpinBox->setValue(stroke.absoluteSpacing.x());
    ui->absoluteSpacingYSpinBox->setValue(stroke.absoluteSpacing.y());
    ui->proportionalSpacingXSpinBox->setValue(stroke.proportionalSpacing.x());
    ui->proportionalSpacingYSpinBox->setValue(stroke.proportionalSpacing.y());
    ui->dabCountSpinBox->setValue(stroke.dabCount);
    for (auto widget : widgets) widget->blockSignals(false);
    updateStroke();
}

void StrokeEditorWidget::updateStroke()
{
    stroke.space = static_cast<Space>(ui->spaceComboBox->currentIndex());
    stroke.continuous = ui->continuousCheckBox->isChecked();
    stroke.absoluteSpacing = {ui->absoluteSpacingXSpinBox->value(), ui->absoluteSpacingYSpinBox->value()};
    stroke.proportionalSpacing = {ui->proportionalSpacingXSpinBox->value(), ui->proportionalSpacingYSpinBox->value()};
    stroke.dabCount = ui->dabCountSpinBox->value();

    emit strokeChanged(stroke);
}

} // namespace GfxPaint
