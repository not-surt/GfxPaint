#ifndef COLOURSPACEPLANESETTINGSWIDGET_H
#define COLOURSPACEPLANESETTINGSWIDGET_H

#include <QWidget>

namespace GfxPaint {

namespace Ui {
class ColourSpacePlaneSettingsWidget;
}

class ColourSpacePlaneSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColourSpacePlaneSettingsWidget(QWidget *parent = nullptr);
    ~ColourSpacePlaneSettingsWidget();

private:
    Ui::ColourSpacePlaneSettingsWidget *ui;
};

} // namespace GfxPaint

#endif // COLOURSPACEPLANESETTINGSWIDGET_H
