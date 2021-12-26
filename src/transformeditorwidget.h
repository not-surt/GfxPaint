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

    const Mat4 &transform() const;
    void setTransform(const Mat4 &transform);

    EditingContext::TransformTarget transformTarget() const;
    void setTransformTarget(const EditingContext::TransformTarget transformTarget);

signals:
    void transformChanged(const GfxPaint::Mat4 &transform);
    void transformTargetChanged(const GfxPaint::EditingContext::TransformTarget transformTarget);

private:
    Ui::TransformEditorWidget *ui;

    TransformModel model;
    EditingContext::TransformTarget m_transformTarget;
};

} // namespace GfxPaint

#endif // TRANSFORMEDITORWIDGET_H
