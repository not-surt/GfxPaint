#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>

#include "documentmanager.h"

class QSettings;

namespace GfxPaint {

class MainWindow;

class SessionManager : public QObject
{
    Q_OBJECT

public:
    SessionManager();
    virtual ~SessionManager();

    const QString &sessionFilename() const;
    void setSessionFilename(const QString &sessionFilename);
    const QList<MainWindow *> &windows() const;

    void readSession(QSettings &settings);
    void writeSession(QSettings &settings) const;
    void updateWindowActivationOrder(MainWindow *const window);
    void closeSession();
    bool openSession(const QString &filename);
    bool saveSession(QString filename);
    void newSession();
    MainWindow *newWindow();
    void deleteWindow(MainWindow *const window);

    static const QString sessionDefaultFilename;

protected:
    virtual bool eventFilter(QObject *const watched, QEvent *const event) override;

    QString m_sessionFilename;
    QList<MainWindow *> m_windows;
    QList<MainWindow *> windowActivationOrder;
};

} // namespace GfxPaint

#endif // SESSIONMANAGER_H
