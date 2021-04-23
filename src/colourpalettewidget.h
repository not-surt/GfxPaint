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
    void setColumnCount(const int columnCount);
    void setFitColumnCount(const bool fitColumnCount);
    void setSwatchSize(const QSize &swatchSize);
    void setFitSwatchSize(const bool fitColumnCount);

signals:
    void colourPicked(const Colour &colour);

protected:
    virtual bool event(QEvent *const event) override;
    virtual void initializeGL() override;
    virtual void render() override;
    virtual int heightForWidth(const int width) const override;
    virtual bool hasHeightForWidth() const override { return true/*m_fitColumnCount || m_fitSwatchSize*/; }

    int columnCountForWidth(const int width) const {
        return (m_fitColumnCount ? width / m_swatchSize.width() : m_columnCount);
    }
    QSize cellsForWidth(const int width) const {
        const int actualColumnCount = columnCountForWidth(width);
        const int actualRowCount = (m_palette ? (m_palette->width() + actualColumnCount - 1) / actualColumnCount : 0);
        return QSize(actualColumnCount, actualRowCount);
    }
    QSize cells() const;
    void updatePaletteLayout();

    int m_columnCount;
    bool m_fitColumnCount;
    QSize m_swatchSize;
    bool m_fitSwatchSize;

    Index leftIndex, rightIndex;
    Index dragStartIndex, dragEndIndex;

    ColourPaletteProgram *program;
    VertexColourModelProgram *markerProgram;
    ColourPalettePickProgram *pickProgram;
    ColourPaletteProgram *selectionProgram;

    const Buffer *m_palette;
    Buffer *m_selection;
    Buffer *m_ordering;    
};

} // namespace GfxPaint

#endif // COLOURPALETTEWIDGET_H
