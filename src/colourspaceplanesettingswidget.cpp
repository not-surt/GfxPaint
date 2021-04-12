#include "colourspaceplanesettingswidget.h"
#include "ui_colourspaceplanesettingswidget.h"

namespace GfxPaint {

ColourSpacePlaneSettingsWidget::ColourSpacePlaneSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourSpacePlaneSettingsWidget)
{
    ui->setupUi(this);
}

ColourSpacePlaneSettingsWidget::~ColourSpacePlaneSettingsWidget()
{
    delete ui;
}

} // namespace GfxPaint
