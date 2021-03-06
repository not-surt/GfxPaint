#ifndef COLOURSLIDERWIDGET_H
#define COLOURSLIDERWIDGET_H

#include "renderedwidget.h"

#include "rendermanager.h"

namespace GfxPaint {

class ColourSliderWidget : public GfxPaint::RenderedWidget
{
    Q_OBJECT

public:
    explicit ColourSliderWidget(const ColourSpace colourSpace, const int component, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent = nullptr);
    explicit ColourSliderWidget(QWidget *const parent = nullptr);
    virtual ~ColourSliderWidget() override;

    virtual QSize sizeHint() const override { return QSize(128, 32); }
    virtual QSize minimumSizeHint() const override { return QSize(64, 16); }

    QColor colour() const { return m_colour; }
    qreal pos() const { return m_pos; }

public slots:
    void setColour(const QColor &colour);
    void setPalette(const Buffer *const palette);
    void setPos(const qreal pos);

signals:
    void colourChanged(const QColor &colour);
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

    ColourSliderProgram *program;
    ColourSliderPickProgram *pickProgram;
    ModelProgram *markerProgram;

    qreal m_pos;
    QColor m_colour;
    const Buffer *m_palette;
    Model *markerModel;

    static const QVector<GLfloat> markerVertices;
    static const QVector<GLushort> markerIndices;
    static const QVector<GLushort> markerElements;
};

} // namespace GfxPaint

#endif // COLOURSLIDERWIDGET_H
