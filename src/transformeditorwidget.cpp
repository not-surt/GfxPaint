#include "transformeditorwidget.h"
#include "ui_transformeditorwidget.h"

namespace GfxPaint {

TransformEditorWidget::TransformEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransformEditorWidget),
    model(),
    m_transformTarget(EditingContext::TransformTarget::View)
{
    ui->setupUi(this);
    ui->transformTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->transformTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->transformTableView->setModel(&model);

    QObject::connect(ui->modeComboBox, qOverload<int>(&QComboBox::activated), this, [this](const int transformTarget){
        setTransformTarget(static_cast<EditingContext::TransformTarget>(transformTarget));
    });
    QObject::connect(ui->identityButton, &QPushButton::clicked, this, [this](){
        model.setTransform(Mat4());
    });
    QObject::connect(&model, &TransformModel::transformChanged, this, &TransformEditorWidget::transformChanged);
}

TransformEditorWidget::~TransformEditorWidget()
{
    delete ui;
}

const Mat4 &TransformEditorWidget::transform() const
{
    return model.transform();
}

void TransformEditorWidget::setTransform(const Mat4 &transform)
{
    model.setTransform(transform);
}

EditingContext::TransformTarget TransformEditorWidget::transformTarget() const
{
    return m_transformTarget;
}

void TransformEditorWidget::setTransformTarget(const EditingContext::TransformTarget transformTarget)
{
    if (m_transformTarget != transformTarget) {
        m_transformTarget = transformTarget;
        ui->modeComboBox->setCurrentIndex(static_cast<int>(m_transformTarget));
        emit transformTargetChanged(m_transformTarget);
    }
}

} // namespace GfxPaint
