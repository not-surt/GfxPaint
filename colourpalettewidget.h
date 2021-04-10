#ifndef COLOURPALETTEWIDGET_H
#define COLOURPALETTEWIDGET_H

#include "renderedwidget.h"

#include "rendermanager.h"

namespace GfxPaint {

class ColourPaletteWidget : public GfxPaint::RenderedWidget
{
    Q_OBJECT

public:
    explicit ColourPaletteWidget(QWidget *const parent = nullptr);
    virtual ~ColourPaletteWidget() override;

    virtual QSize sizeHint() const override { return QSize(128, 128); }
    virtual QSize minimumSizeHint() const override { return QSize(64, 64); }

public slots:
    void setPalette(const GfxPaint::Buffer *const palette);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseReleaseEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseMoveEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void resizeGL(int w, int h) override;
    virtual void render() override;

    void mouseEvent(QMouseEvent *event);

    QSize swatchSize;
    int columns;

    ColourPaletteProgram *program;

    const Buffer *m_palette;
};

} // namespace GfxPaint

#endif // COLOURPALETTEWIDGET_H
