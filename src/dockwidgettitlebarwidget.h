#ifndef DOCKWIDGETTITLEBARWIDGET_H
#define DOCKWIDGETTITLEBARWIDGET_H

#include <QWidget>

#include <QLabel>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QWidgetAction>

namespace GfxPaint {

class DockWidgetTitlebarWidget : public QWidget {
    Q_OBJECT
public:
    DockWidgetTitlebarWidget(QWidget *const parent = nullptr);
    QAction *menuAction() { return m_menuAction; }
    QWidgetAction *popupAction() { return m_popupAction; }
    QAction *floatAction() { return m_floatAction; }
    QAction *closeAction() { return m_closeAction; }
public slots:
    void setTitle(const QString &text) { titleLabel->setText(text); }
    void setMenu(QMenu *const menu);
    void setPopup(QWidget *const popup);
protected:
    QLabel *titleLabel;
    QToolButton *menuToolButton;
    QToolButton *popupToolButton;
    QAction *m_menuAction;
    QWidgetAction *m_popupAction;
    QAction *m_floatAction;
    QAction *m_closeAction;
};

} // namespace GfxPaint

#endif // DOCKWIDGETTITLEBARWIDGET_H
