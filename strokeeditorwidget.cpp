#include "strokeeditorwidget.h"
#include "ui_strokeeditorwidget.h"

namespace GfxPaint {

StrokeEditorWidget::StrokeEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StrokeEditorWidget),
    stroke()
{
    ui->setupUi(this);

    QObject::connect(ui->spaceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->metricComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->continuousCheckBox, &QCheckBox::toggled, this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->absoluteSpacingWidthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->absoluteSpacingHeightSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->proportionalSpacingWidthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->proportionalSpacingHeightSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
    QObject::connect(ui->dabCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &StrokeEditorWidget::updateStroke);
}

StrokeEditorWidget::~StrokeEditorWidget()
{
    delete ui;
}

void StrokeEditorWidget::setStroke(const Brush::Stroke &stroke)
{
    if (this->stroke != stroke) {
        this->stroke = stroke;
        updateWidgets();
    }
}

void StrokeEditorWidget::updateWidgets()
{
    const QWidgetList widgets = {
        ui->spaceComboBox, ui->metricComboBox,
        ui->continuousCheckBox,
        ui->absoluteSpacingWidthSpinBox, ui->absoluteSpacingHeightSpinBox,
        ui->proportionalSpacingWidthSpinBox, ui->proportionalSpacingHeightSpinBox,
        ui->dabCountSpinBox,
    };
    for (auto widget : widgets) widget->blockSignals(true);
    ui->spaceComboBox->setCurrentIndex(static_cast<int>(stroke.space));
    ui->metricComboBox->setCurrentIndex(stroke.metric);
    ui->continuousCheckBox->setChecked(stroke.continuous);
    ui->absoluteSpacingWidthSpinBox->setValue(stroke.absoluteSpacing.x());
    ui->absoluteSpacingHeightSpinBox->setValue(stroke.absoluteSpacing.y());
    ui->proportionalSpacingWidthSpinBox->setValue(stroke.proportionalSpacing.x());
    ui->proportionalSpacingHeightSpinBox->setValue(stroke.proportionalSpacing.y());
    ui->dabCountSpinBox->setValue(stroke.dabCount);
    for (auto widget : widgets) widget->blockSignals(false);
    updateStroke();
}

void StrokeEditorWidget::updateStroke()
{
    stroke.space = static_cast<Space>(ui->spaceComboBox->currentIndex());
    stroke.metric = ui->spaceComboBox->currentIndex();
    stroke.continuous = ui->continuousCheckBox->isChecked();
    stroke.absoluteSpacing = {static_cast<float>(ui->absoluteSpacingWidthSpinBox->value()), static_cast<float>(ui->absoluteSpacingHeightSpinBox->value())};
    stroke.proportionalSpacing = {static_cast<float>(ui->proportionalSpacingWidthSpinBox->value()), static_cast<float>(ui->proportionalSpacingHeightSpinBox->value())};
    stroke.dabCount = ui->dabCountSpinBox->value();

    emit strokeChanged(stroke);
}

} // namespace GfxPaint
