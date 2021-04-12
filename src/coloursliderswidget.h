#ifndef COLOURSLIDERSWIDGET_H
#define COLOURSLIDERSWIDGET_H

#include <QWidget>

#include "types.h"
#include "buffer.h"

namespace GfxPaint {

namespace Ui {
class ColourSlidersWidget;
}
class ColourConversionProgram;

class ColourSlidersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColourSlidersWidget(QWidget *parent = nullptr);
    ~ColourSlidersWidget();

    Colour colour() const;

public slots:
    void setColour(const GfxPaint::Colour &colour);
    void setPalette(const GfxPaint::Buffer *const palette);

signals:
    void colourChanged(const GfxPaint::Colour &colour);

private:
    void updateSliderColours();
    void updateSliderPositions();
    void updateColourFromSliders();
    void updateWidgets();
    void updateColourSpace();

    Ui::ColourSlidersWidget *ui;

    ColourConversionProgram *fromRGBConversionProgram;
    ColourConversionProgram *toRGBConversionProgram;

    Colour m_colour;
    const Buffer *m_palette;
};

} // namespace GfxPaint

#endif // COLOURSLIDERSWIDGET_H
