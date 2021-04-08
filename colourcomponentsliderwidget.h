#ifndef COLOURCOMPONENTSLIDERWIDGET_H
#define COLOURCOMPONENTSLIDERWIDGET_H

#include "renderedwidget.h"

#include "rendermanager.h"

namespace GfxPaint {

class ColourComponentSliderWidget : public GfxPaint::RenderedWidget
{
    Q_OBJECT

public:
    explicit ColourComponentSliderWidget(const ColourSpace colourSpace, const int component, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent = nullptr);
    explicit ColourComponentSliderWidget(QWidget *const parent = nullptr);
    virtual ~ColourComponentSliderWidget() override;

    virtual QSize sizeHint() const override { return QSize(128, 32); }
    virtual QSize minimumSizeHint() const override { return QSize(64, 16); }

    const Colour &colour() const { return m_colour; }
    qreal pos() const { return m_pos; }

public slots:
    void setColour(const GfxPaint::Colour &colour);
    void setPalette(const GfxPaint::Buffer *const palette);
    void setPos(const qreal pos);

signals:
    void colourChanged(const GfxPaint::Colour &colour);
    void posChanged(const qreal pos);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseReleaseEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseMoveEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void resizeGL(int w, int h) override;
    virtual void render() override;

    void mouseEvent(QMouseEvent *event);

    ColourSpace colourSpace;
    int component;
    bool quantise;
    Buffer::Format quantisePaletteFormat;

    ColourPlaneProgram *program;
    ColourPlanePickProgram *pickProgram;
    ModelProgram *markerProgram;

    qreal m_pos;
    Colour m_colour;
    const Buffer *m_palette;
};

} // namespace GfxPaint

#endif // COLOURCOMPONENTSLIDERWIDGET_H
