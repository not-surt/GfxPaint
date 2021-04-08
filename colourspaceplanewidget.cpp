#include "colourspaceplanewidget.h"
#include "ui_colourspaceplanewidget.h"

#include "colourcomponentsplanewidget.h"
#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourSpacePlaneWidget::ColourSpacePlaneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourSpacePlaneWidget), plane(nullptr), alphaSlider(nullptr),
    fromRGBConversionProgram(nullptr), toRGBConversionProgram(nullptr),
    m_colour{},
    m_palette(nullptr)
{
    ui->setupUi(this);

    setColour(m_colour);
    updateWidgets();

    QObject::connect(ui->colourSpaceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->xComponentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->yComponentComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->alphaCheckBox, &QCheckBox::toggled, this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->quantiseCheckBox, &QCheckBox::toggled, this, &ColourSpacePlaneWidget::updateWidgets);
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
    if (ui->alphaCheckBox->isChecked()) {
        alphaSlider->blockSignals(true);
        alphaSlider->setColour(m_colour);
        alphaSlider->blockSignals(false);
    }
}

void ColourSpacePlaneWidget::updateWidgetPositions()
{
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    Colour spaceColour = fromRGBConversionProgram->convert(m_colour);
    plane->blockSignals(true);
    plane->setPos(QVector2D(clamp(0.0f, 1.0f, spaceColour.rgba[ui->xComponentComboBox->currentIndex()]), clamp(0.0f, 1.0f, spaceColour.rgba[ui->yComponentComboBox->currentIndex()])));
    plane->blockSignals(false);
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        alphaSlider->blockSignals(true);
        alphaSlider->setPos(clamp(0.0f, 1.0f, spaceColour.rgba[alphaComponent]));
        alphaSlider->blockSignals(false);
    }
}

void ColourSpacePlaneWidget::updateColourFromWidgets()
{
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    Colour spaceColour = m_colour;
    spaceColour.rgba[ui->xComponentComboBox->currentIndex()] = clamp(0.0f, 1.0f, plane->pos().x());
    spaceColour.rgba[ui->yComponentComboBox->currentIndex()] = clamp(0.0f, 1.0f, plane->pos().y());
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        spaceColour.rgba[alphaComponent] = clamp(0.0, 1.0, alphaSlider->pos());
    }
    m_colour = toRGBConversionProgram->convert(spaceColour);
}

void ColourSpacePlaneWidget::updateWidgets()
{
    while (ui->colourSpacePlaneLayout->count() > 0) {
        ui->colourSpacePlaneLayout->takeAt(0)->widget()->deleteLater();
    }
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    {
        const QList<Program *> oldPrograms = {fromRGBConversionProgram, toRGBConversionProgram};
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        fromRGBConversionProgram = new ColourConversionProgram(ColourSpace::RGB, colourSpace);
        toRGBConversionProgram = new ColourConversionProgram(colourSpace, ColourSpace::RGB);
        qDeleteAll(oldPrograms);
    }
    plane = new ColourComponentsPlaneWidget(colourSpace, ui->xComponentComboBox->currentIndex(), ui->yComponentComboBox->currentIndex(), ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
    ui->colourSpacePlaneLayout->addWidget(plane);
    plane->setPalette(m_palette);
    QObject::connect(plane, &ColourComponentsPlaneWidget::posChanged, [this](const QVector2D &pos){
        updateColourFromWidgets();
        updateWidgetColours();
        emit colourChanged(m_colour);
    });
    const int xComponentIndex = ui->xComponentComboBox->currentIndex();
    const int yComponentIndex = ui->yComponentComboBox->currentIndex();
    ui->xComponentComboBox->blockSignals(true);
    ui->yComponentComboBox->blockSignals(true);
    ui->xComponentComboBox->clear();
    ui->yComponentComboBox->clear();
    for (int i = 0; i < colourSpaceInfo[colourSpace].componentCount; ++i) {
        ui->xComponentComboBox->addItem(colourSpaceInfo[colourSpace].componentLabels[i], i);
        ui->yComponentComboBox->addItem(colourSpaceInfo[colourSpace].componentLabels[i], i);
    }
    ui->xComponentComboBox->setCurrentIndex(xComponentIndex);
    ui->yComponentComboBox->setCurrentIndex(yComponentIndex);
    ui->xComponentComboBox->blockSignals(false);
    ui->yComponentComboBox->blockSignals(false);
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        alphaSlider = new ColourComponentSliderWidget(colourSpace, alphaComponent, ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
        ui->colourSpacePlaneLayout->addWidget(alphaSlider);
        alphaSlider->setPalette(m_palette);
        QObject::connect(alphaSlider, &ColourComponentSliderWidget::posChanged, [this](const GLfloat pos){
            updateColourFromWidgets();
            updateWidgetColours();
            emit colourChanged(m_colour);
        });
    }
    updateWidgetPositions();
    updateWidgetColours();
}

} // namespace GfxPaint
