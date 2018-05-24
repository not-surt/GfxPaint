#ifndef COLOURSLIDERWIDGET_H
#define COLOURSLIDERWIDGET_H

#include "renderedwidget.h"

#include "rendermanager.h"

namespace GfxPaint {

class ColourSliderWidget : public GfxPaint::RenderedWidget
{
    Q_OBJECT

public:
    explicit ColourSliderWidget(const ColourSpace colourSpace, const int component, QWidget *const parent = nullptr);
    explicit ColourSliderWidget(QWidget *const parent = nullptr);
    virtual ~ColourSliderWidget() override;

    virtual QSize sizeHint() const override { return QSize(128, 32); }
    virtual QSize minimumSizeHint() const override { return QSize(64, 16); }

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

    ColourSpace colourSpace;
    int component;

    ColourSliderProgram *program;

    QColor m_colour;
};

} // namespace GfxPaint

#endif // COLOURSLIDERWIDGET_H
