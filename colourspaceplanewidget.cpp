#include "colourspaceplanewidget.h"
#include "ui_colourspaceplanewidget.h"

#include "colourcomponentsplanewidget.h"
#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourSpacePlaneWidget::ColourSpacePlaneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourSpacePlaneWidget), plane(nullptr), zComponentSlider(nullptr), alphaSlider(nullptr),
    fromRGBConversionProgram(nullptr), toRGBConversionProgram(nullptr),
    m_colour{},
    m_palette(nullptr),
    xComponent(0), yComponent(1), zComponent(2)
{
    ui->setupUi(this);

    ui->colourSpaceComboBox->blockSignals(true);
    ui->colourSpaceComboBox->clear();
    auto iterator = std::begin(colourSpaceInfo);
    while (iterator != std::end(colourSpaceInfo)) {
        ui->colourSpaceComboBox->insertItem(static_cast<int>(iterator.key()), iterator.value().label);
        ++iterator;
    }
    ui->colourSpaceComboBox->setCurrentIndex(0);
    ui->colourSpaceComboBox->blockSignals(false);

    QObject::connect(ui->colourSpaceComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateColourSpace);
    QObject::connect(ui->xYComponentsComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->alphaCheckBox, &QCheckBox::toggled, this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->quantiseCheckBox, &QCheckBox::toggled, this, &ColourSpacePlaneWidget::updateWidgets);

    setColour(m_colour);
    updateColourSpace();
}

ColourSpacePlaneWidget::~ColourSpacePlaneWidget()
{
    delete ui;

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        delete fromRGBConversionProgram;
        delete toRGBConversionProgram;
    }
}

Colour ColourSpacePlaneWidget::colour() const
{
    return m_colour;
}

void ColourSpacePlaneWidget::setColour(const Colour &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        updateWidgetColours();
        updateWidgetPositions();
        emit colourChanged(m_colour);
    }
}

void ColourSpacePlaneWidget::setPalette(const Buffer *const palette)
{
    if (m_palette != palette) {
        m_palette = palette;
        if (ui->quantiseCheckBox->isChecked()) updateWidgets();
    }
}

void ColourSpacePlaneWidget::updateWidgetColours()
{
    plane->blockSignals(true);
    plane->setColour(m_colour);
    plane->blockSignals(false);
    zComponentSlider->blockSignals(true);
    zComponentSlider->setColour(m_colour);
    zComponentSlider->blockSignals(false);
    if (ui->alphaCheckBox->isChecked()) {
        alphaSlider->blockSignals(true);
        alphaSlider->setColour(m_colour);
        alphaSlider->blockSignals(false);
    }
}

void ColourSpacePlaneWidget::updateWidgetPositions()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    Colour spaceColour = fromRGBConversionProgram->convert(m_colour);
    plane->blockSignals(true);
    plane->setPos(QVector2D(spaceColour.rgba[xComponent], spaceColour.rgba[yComponent]));
    plane->blockSignals(false);
    zComponentSlider->blockSignals(true);
    zComponentSlider->setPos(spaceColour.rgba[zComponent]);
    zComponentSlider->blockSignals(false);
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        alphaSlider->blockSignals(true);
        alphaSlider->setPos(clamp(0.0f, 1.0f, spaceColour.rgba[alphaComponent]));
        alphaSlider->blockSignals(false);
    }
}

void ColourSpacePlaneWidget::updateColourFromWidgets()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    Colour spaceColour = m_colour;
    spaceColour.rgba[xComponent] = clamp(0.0f, 1.0f, plane->pos().x());
    spaceColour.rgba[yComponent] = clamp(0.0f, 1.0f, plane->pos().y());
    spaceColour.rgba[zComponent] = clamp(0.0f, 1.0f, (float)zComponentSlider->pos());
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        spaceColour.rgba[alphaComponent] = clamp(0.0f, 1.0f, (float)alphaSlider->pos());
    }
    m_colour = toRGBConversionProgram->convert(spaceColour);
}

void ColourSpacePlaneWidget::updateWidgets()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    while (ui->colourSpacePlaneLayout->count() > 0) {
        ui->colourSpacePlaneLayout->takeAt(0)->widget()->deleteLater();
    }

    const QPoint xYComponents = ui->xYComponentsComboBox->currentData().isNull() ? QPoint(0, 1) : ui->xYComponentsComboBox->currentData().toPoint();
    xComponent = ui->xYComponentsComboBox->currentData().toPoint().x();
    yComponent = ui->xYComponentsComboBox->currentData().toPoint().y();
    zComponent = 3 - xYComponents.x() - xYComponents.y();

    auto updateFromChildWidget = [this](){
        updateColourFromWidgets();
        updateWidgetColours();
        emit colourChanged(m_colour);
    };
    plane = new ColourComponentsPlaneWidget(colourSpace, xComponent, yComponent, ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
    ui->colourSpacePlaneLayout->addWidget(plane);
    plane->setPalette(m_palette);
    QObject::connect(plane, &ColourComponentsPlaneWidget::posChanged, updateFromChildWidget);
    zComponentSlider = new ColourComponentSliderWidget(colourSpace, zComponent, ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
    ui->colourSpacePlaneLayout->addWidget(zComponentSlider);
    zComponentSlider->setPalette(m_palette);
    QObject::connect(zComponentSlider, &ColourComponentSliderWidget::posChanged, updateFromChildWidget);
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        alphaSlider = new ColourComponentSliderWidget(colourSpace, alphaComponent, ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
        ui->colourSpacePlaneLayout->addWidget(alphaSlider);
        alphaSlider->setPalette(m_palette);
        QObject::connect(alphaSlider, &ColourComponentSliderWidget::posChanged, updateFromChildWidget);
    }
    updateWidgetPositions();
    updateWidgetColours();
}

void ColourSpacePlaneWidget::updateColourSpace()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    {
        const QList<Program *> oldPrograms = {fromRGBConversionProgram, toRGBConversionProgram};
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        fromRGBConversionProgram = new ColourConversionProgram(ColourSpace::RGB, colourSpace);
        toRGBConversionProgram = new ColourConversionProgram(colourSpace, ColourSpace::RGB);
        qDeleteAll(oldPrograms);
    }
    const int componentIndex = (ui->xYComponentsComboBox->currentIndex() >= 0 ? ui->xYComponentsComboBox->currentIndex() : 0);
    ui->xYComponentsComboBox->blockSignals(true);
    ui->xYComponentsComboBox->clear();
    for (int x = 0; x < colourSpaceInfo[colourSpace].componentCount; ++x) {
        for (int y = 0; y < colourSpaceInfo[colourSpace].componentCount; ++y) {
            if (x != y) {
                const QString text = colourSpaceInfo[colourSpace].componentLabels[x] + "/" + colourSpaceInfo[colourSpace].componentLabels[y];
                ui->xYComponentsComboBox->addItem(text, QPoint(x, y));
            }
        }
    }
    ui->xYComponentsComboBox->setCurrentIndex(componentIndex);
    ui->xYComponentsComboBox->blockSignals(false);

    updateWidgets();
}

} // namespace GfxPaint
