#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>

#include "dockwidgettitlebarwidget.h"

class QMenu;

namespace GfxPaint {

class DockWidget : public QDockWidget
{
    Q_OBJECT
public:
    DockWidget(QWidget *const parent = nullptr, const Qt::WindowFlags &flags = Qt::WindowFlags());

    void setMenu(QMenu *const menu);
    void setPopup(QWidget *const popup);
protected:
    DockWidgetTitlebarWidget *titleBar;
    QMenu *m_menu;
    QWidget *m_popup;
};

} // namespace GfxPaint

#endif // DOCKWIDGET_H
