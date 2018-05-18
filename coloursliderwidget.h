#ifndef COLOURSLIDERWIDGET_H
#define COLOURSLIDERWIDGET_H

#include "renderedwidget.h"

#include "rendermanager.h"

namespace GfxPaint {

class ColourSliderWidget : public GfxPaint::RenderedWidget
{
    Q_OBJECT

public:
    explicit ColourSliderWidget(QWidget *const parent = nullptr);
    virtual ~ColourSliderWidget();

    virtual QSize sizeHint() const override { return QSize(64, 64); }
    virtual QSize minimumSizeHint() const override { return QSize(64, 64); }

    QColor colour() const;

public slots:
    void setColour(const QColor &colour);

signals:
    void colourChanged(const QColor &colour);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseReleaseEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void mouseMoveEvent(QMouseEvent *event) override { mouseEvent(event); }
    virtual void resizeGL(int w, int h) override;
    virtual void render() override;

    bool mouseEvent(QMouseEvent *event);

    ColourSliderProgram *program;

    QColor m_colour;
};

} // namespace GfxPaint

#endif // COLOURSLIDERWIDGET_H
