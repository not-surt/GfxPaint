#ifndef COLOURCOMPONENTSPLANEWIDGET_H
#define COLOURCOMPONENTSPLANEWIDGET_H

#include "renderedwidget.h"

#include "rendermanager.h"

namespace GfxPaint {

class ColourComponentsPlaneWidget : public GfxPaint::RenderedWidget
{
    Q_OBJECT

public:
    explicit ColourComponentsPlaneWidget(const ColourSpace colourSpace, const int xComponent, const int yComponent, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent = nullptr);
    explicit ColourComponentsPlaneWidget(QWidget *const parent = nullptr);
    virtual ~ColourComponentsPlaneWidget() override;

    virtual QSize sizeHint() const override { return QSize(xComponent >= 0 ? 128 : 32, yComponent >= 0 ? 128 : 32); }
    virtual QSize minimumSizeHint() const override { return QSize(xComponent >= 0 ? 64 : 16, yComponent >= 0 ? 64 : 16); }

    const Colour &colour() const { return m_colour; }
    const Vec2 &pos() const { return m_pos; }

public slots:
    void setColour(const GfxPaint::Colour &colour);
    void setPalette(const GfxPaint::Buffer *const palette);
    void setPos(const Vec2 &pos);

signals:
    void posChanged(const Vec2 &pos);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseReleaseEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseMoveEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void initializeGL() override;
    virtual void render() override;

    void mouseEvent(QMouseEvent *event);

    ColourSpace colourSpace;
    int xComponent, yComponent;
    bool quantise;
    Buffer::Format quantisePaletteFormat;

    ColourPlaneProgram *program;
    VertexColourModelProgram *markerProgram;

    Vec2 m_pos;
    Colour m_colour;
    const Buffer *m_palette;
};

} // namespace GfxPaint

#endif // COLOURCOMPONENTSPLANEWIDGET_H
