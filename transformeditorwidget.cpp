#include "transformeditorwidget.h"
#include "ui_transformeditorwidget.h"

namespace GfxPaint {

TransformEditorWidget::TransformEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransformEditorWidget),
    model(),
    m_transformMode(TransformMode::View)
{
    ui->setupUi(this);
    ui->transformTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->transformTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->transformTableView->setModel(&model);

    QObject::connect(ui->modeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](const int transformMode){
        setTransformMode(static_cast<TransformMode>(transformMode));
    });
    QObject::connect(ui->identityButton, &QPushButton::clicked, this, [this](){
        model.setTransform(QTransform());
    });
    QObject::connect(&model, &TransformModel::transformChanged, this, &TransformEditorWidget::transformChanged);
}

TransformEditorWidget::~TransformEditorWidget()
{
    delete ui;
}

const QTransform &TransformEditorWidget::transform() const
{
    return model.transform();
}

void TransformEditorWidget::setTransform(const QTransform &transform)
{
    model.setTransform(transform);
}

TransformMode TransformEditorWidget::transformMode() const
{
    return m_transformMode;
}

void TransformEditorWidget::setTransformMode(const TransformMode transformMode)
{
    if (m_transformMode != transformMode) {
        m_transformMode = transformMode;
        ui->modeComboBox->setCurrentIndex(static_cast<int>(m_transformMode));
        emit transformModeChanged(m_transformMode);
    }
}

} // namespace GfxPaint
