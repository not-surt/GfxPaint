#ifndef BRUSHVIEWWIDGET_H
#define BRUSHVIEWWIDGET_H

#include "renderedwidget.h"

#include "brush.h"
#include "program.h"
#include "rendermanager.h"

namespace GfxPaint {

class BrushViewWidget : public RenderedWidget
{
public:
    explicit BrushViewWidget(QWidget *const parent = nullptr);
    virtual ~BrushViewWidget();

    virtual QSize sizeHint() const override { return QSize(128, 128); }
    virtual QSize minimumSizeHint() const override { return QSize(32, 32); }

public slots:
    void setBrush(const GfxPaint::Brush &brush);
    void setColour(const GfxPaint::Colour &colour)
    {
        if (this->colour != colour) {
            this->colour = colour;
            update();
        }
    }

protected:
    virtual void initializeGL() override;
    virtual void render() override;

    Brush brush;
    Colour colour;

    DabProgram *program;
};

} // namespace GfxPaint

#endif // BRUSHVIEWWIDGET_H
