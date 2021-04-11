#ifndef COLOURPLANEWIDGET_H
#define COLOURPLANEWIDGET_H

#include <QWidget>

#include "types.h"
#include "buffer.h"

namespace GfxPaint {

namespace Ui {
class ColourPlaneWidget;
}
class ColourComponentsPlaneWidget;
class ColourConversionProgram;

class ColourPlaneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColourPlaneWidget(QWidget *parent = nullptr);
    ~ColourPlaneWidget();

    Colour colour() const;

public slots:
    void setColour(const GfxPaint::Colour &colour);
    void setPalette(const GfxPaint::Buffer *const palette);

signals:
    void colourChanged(const GfxPaint::Colour &colour);

private:
    void updateWidgetColours();
    void updateWidgetPositions();
    void updateColourFromWidgets();
    void updateWidgets();
    void updateColourSpace();

    Ui::ColourPlaneWidget *ui;
    ColourComponentsPlaneWidget *plane;
    ColourComponentsPlaneWidget *zComponentSlider;
    ColourComponentsPlaneWidget *alphaSlider;

    ColourConversionProgram *fromRGBConversionProgram;
    ColourConversionProgram *toRGBConversionProgram;

    Colour m_colour;
    const Buffer *m_palette;
    int xComponent, yComponent, zComponent;
};

} // namespace GfxPaint

#endif // COLOURPLANEWIDGET_H
