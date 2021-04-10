#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>

#include "dockwidgettitlebarwidget.h"

namespace GfxPaint {

class DockWidget : public QDockWidget
{
    Q_OBJECT
public:
    DockWidget(QWidget *const parent = nullptr, const Qt::WindowFlags &flags = Qt::WindowFlags());

    void setMenu(QMenu *const menu) {
        m_menu = menu;
        titleBar->menuAction()->setMenu(m_menu);
        titleBar->menuAction()->setEnabled(m_menu != nullptr);
    }
    void setPopup(QWidget *const popup) {
        m_popup = popup;
        titleBar->popupAction()->setDefaultWidget(m_popup);
        titleBar->popupAction()->setEnabled(m_menu != nullptr);
    }
protected:
    DockWidgetTitlebarWidget *titleBar;
    QMenu *m_menu;
    QWidget *m_popup;
};

} // namespace GfxPaint

#endif // DOCKWIDGET_H
