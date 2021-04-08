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

    virtual QSize sizeHint() const override { return QSize(128, 32); }
    virtual QSize minimumSizeHint() const override { return QSize(64, 16); }

    const Colour &colour() const { return m_colour; }
    const QVector2D &pos() const { return m_pos; }

public slots:
    void setColour(const GfxPaint::Colour &colour);
    void setPalette(const GfxPaint::Buffer *const palette);
    void setPos(const QVector2D &pos);

signals:
    void colourChanged(const GfxPaint::Colour &colour);
    void posChanged(const QVector2D &pos);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseReleaseEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseMoveEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void resizeGL(int w, int h) override;
    virtual void render() override;

    void mouseEvent(QMouseEvent *event);

    ColourSpace colourSpace;
    bool useXAxis, useYAxis;
    int xComponent, yComponent;
    bool quantise;
    Buffer::Format quantisePaletteFormat;

    ColourPlaneProgram *program;
    ColourPlanePickProgram *pickProgram;
    ModelProgram *markerProgram;

    QVector2D m_pos;
    Colour m_colour;
    const Buffer *m_palette;
    Model *markerModel;

    static const QVector<GLsizei> markerAttributeSizes;
    static const QVector<GLfloat> markerVertices;
    static const QVector<GLushort> markerIndices;
    static const QVector<GLushort> markerElementSizes;
};

} // namespace GfxPaint

#endif // COLOURCOMPONENTSPLANEWIDGET_H
