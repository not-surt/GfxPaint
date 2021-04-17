#ifndef TRANSFORMEDITORWIDGET_H
#define TRANSFORMEDITORWIDGET_H

#include <QWidget>

#include <QAbstractTableModel>
#include "editor.h"
#include "transformmodel.h"

namespace GfxPaint {

namespace Ui {
class TransformEditorWidget;
}

class TransformEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransformEditorWidget(QWidget *parent = nullptr);
    ~TransformEditorWidget();

    const QMatrix4x4 &transform() const;
    void setTransform(const QMatrix4x4 &transform);

    TransformMode transformMode() const;
    void setTransformMode(const TransformMode transformMode);

signals:
    void transformChanged(const QMatrix4x4 &transform);
    void transformModeChanged(const GfxPaint::TransformMode transformMode);

private:
    Ui::TransformEditorWidget *ui;

    TransformModel model;
    TransformMode m_transformMode;
};

} // namespace GfxPaint

#endif // TRANSFORMEDITORWIDGET_H