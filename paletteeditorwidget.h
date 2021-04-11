#ifndef GFXPAINT_PALETTEEDITORWIDGET_H
#define GFXPAINT_PALETTEEDITORWIDGET_H

#include <QWidget>

#include "buffer.h"
#include "types.h"

namespace GfxPaint {

namespace Ui {
class PaletteEditorWidget;
}

class PaletteEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PaletteEditorWidget(QWidget *parent = nullptr);
    ~PaletteEditorWidget();

public slots:
    void setPalette(const GfxPaint::Buffer *const palette);

signals:
    void colourPicked(const Colour &colour);

private:
    Ui::PaletteEditorWidget *ui;
};


} // namespace GfxPaint
#endif // GFXPAINT_PALETTEEDITORWIDGET_H
