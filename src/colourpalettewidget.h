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

    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override { return QSize(64, 64); }

public slots:
    void setPalette(const GfxPaint::Buffer *const palette);
    void setColumnCount(const int columns);
    void setSwatchSize(const QSize &size);

signals:
    void colourPicked(const Colour &colour);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseReleaseEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseMoveEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void initializeGL() override;
    virtual void render() override;

    QSize cells() { return m_palette ? QSize(m_columnCount, (m_palette->size().width() + m_columnCount - 1) / m_columnCount) : QSize(0, 0); }

    void mouseEvent(QMouseEvent *event);

    QSize m_swatchSize;
    int m_columnCount;

    ColourPaletteProgram *program;
    ColourPalettePickProgram *pickProgram;
    ColourPaletteProgram *selectionProgram;

    const Buffer *m_palette;
    Buffer *m_selection;
    Buffer *m_ordering;
};

} // namespace GfxPaint

#endif // COLOURPALETTEWIDGET_H
