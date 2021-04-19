#include "paletteeditorwidget.h"
#include "ui_paletteeditorwidget.h"

#include <QScroller>

namespace GfxPaint {

PaletteEditorWidget::PaletteEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaletteEditorWidget)
{
    ui->setupUi(this);

    QObject::connect(ui->orderingComboBox, qOverload<int>(&QComboBox::activated), this, &PaletteEditorWidget::reorder);
    QObject::connect(ui->colourSpaceComponentComboBox, qOverload<int>(&QComboBox::activated), this, &PaletteEditorWidget::reorder);
    QObject::connect(ui->colourSpaceComboBox, qOverload<int>(&QComboBox::activated), this, &PaletteEditorWidget::updateColourSpaceComponentComboBox);

    ui->colourSpaceComboBox->blockSignals(true);
    ui->colourSpaceComboBox->clear();
    auto iterator = colourSpaceInfo.begin();
    while (iterator != colourSpaceInfo.end()) {
        ui->colourSpaceComboBox->insertItem(static_cast<int>(iterator.key()), iterator.value().label);
        ++iterator;
    }
    ui->colourSpaceComboBox->setCurrentIndex(0);
    ui->colourSpaceComboBox->blockSignals(false);
    updateColourSpaceComponentComboBox();

    QObject::connect(ui->colourPaletteWidget, &ColourPaletteWidget::colourPicked, this, &PaletteEditorWidget::colourPicked);
    QObject::connect(ui->columnCountSpinBox, &QSpinBox::valueChanged, ui->colourPaletteWidget, &ColourPaletteWidget::setColumnCount);
    QObject::connect(ui->fitColumnCountCheckBox, &QCheckBox::toggled, ui->colourPaletteWidget, &ColourPaletteWidget::setFitColumnCount);
    QObject::connect(ui->swatchSizeSpinBox, &QSpinBox::valueChanged, this, [this](const int size){
        ui->colourPaletteWidget->setSwatchSize(QSize(size, size));
    });
    QObject::connect(ui->fitSwatchSizeCheckBox, &QCheckBox::toggled, ui->colourPaletteWidget, &ColourPaletteWidget::setFitSwatchSize);

    ui->scrollArea->installEventFilter(this);
    QScroller *scroller = QScroller::scroller(ui->scrollArea);
    QScrollerProperties scrollerProperties;
    scrollerProperties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    scrollerProperties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    scrollerProperties.setScrollMetric(QScrollerProperties::DecelerationFactor, 10.0);
    scroller->setScrollerProperties(scrollerProperties);
    QScroller::grabGesture(ui->scrollArea, QScroller::MiddleMouseButtonGesture);
}

PaletteEditorWidget::~PaletteEditorWidget()
{
    delete ui;
}

bool PaletteEditorWidget::eventFilter(QObject *const watched, QEvent *const event)
{
    if (watched == ui->scrollArea) {
        if (event->type() == QEvent::QEvent::MouseButtonPress) {

        }
        if (event->type() == QEvent::QEvent::MouseButtonRelease) {

        }
    }
    return false;
}

void PaletteEditorWidget::setPalette(const Buffer *const palette)
{
    ui->colourPaletteWidget->setPalette(palette);
}

void PaletteEditorWidget::reorder()
{

}

void PaletteEditorWidget::updateColourSpaceComponentComboBox()
{
    const int componentIndex = (ui->colourSpaceComponentComboBox->currentIndex() >= 0 ? ui->colourSpaceComponentComboBox->currentIndex() : 0);
    ui->colourSpaceComponentComboBox->blockSignals(true);
    ui->colourSpaceComponentComboBox->clear();
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    for (int i = 0; i < colourSpaceInfo[colourSpace].componentLabels.length(); ++i) {
        ui->colourSpaceComponentComboBox->insertItem(i, colourSpaceInfo[colourSpace].componentLabels[i]);
    }
    ui->colourSpaceComponentComboBox->setCurrentIndex(componentIndex);
    ui->colourSpaceComponentComboBox->blockSignals(false);
}

} // namespace GfxPaint
