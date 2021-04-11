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

signals:
    void colourPicked(const Colour &colour);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseReleaseEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseMoveEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void initializeGL() override;
    virtual void render() override;

    QSize cells() { return m_palette ? QSize(columns, (m_palette->size().width() + columns - 1) / columns) : QSize(0, 0); }

    void mouseEvent(QMouseEvent *event);

    QSize swatchSize;
    int columns;

    ColourPaletteProgram *program;
    ColourPalettePickProgram *pickProgram;
    ColourPaletteProgram *selectionProgram;

    const Buffer *m_palette;
    Buffer *m_selection;
};

} // namespace GfxPaint

#endif // COLOURPALETTEWIDGET_H
