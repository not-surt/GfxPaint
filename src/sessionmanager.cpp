#include "sessionmanager.h"

#include <QSettings>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>

#include "application.h"
#include "mainwindow.h"
#include "scene.h"

namespace GfxPaint {

const QString SessionManager::sessionDefaultFilename = "Default";

SessionManager::SessionManager()
    : m_sessionFilename(),
      m_windows(), windowActivationOrder()
{
}

SessionManager::~SessionManager()
{
}

const QString &SessionManager::sessionFilename() const
{
    return !m_sessionFilename.isEmpty() ? m_sessionFilename : sessionDefaultFilename;
}

void SessionManager::setSessionFilename(const QString &sessionFilename)
{
    m_sessionFilename = sessionFilename;
}

const QList<MainWindow *> &SessionManager::windows() const
{
    return m_windows;
}

void SessionManager::readSession(QSettings &settings)
{
    QList<MainWindow *> windows;
    const int windowCount = settings.beginReadArray("windows");
    for (int i = 0; i < windowCount; ++i) {
        settings.setArrayIndex(i);
        MainWindow *const window = newWindow();
        windows.insert(i, window);
        window->readSettings(settings);
    }
    settings.endArray();

    if (windowCount <= 0) windows.append(newWindow());

    QStringList documentFilenames = {};
    const int documentCount = settings.beginReadArray("documents");
    for (int i = 0; i < documentCount; ++i) {
        settings.setArrayIndex(i);
        if (settings.contains("filename") && !settings.value("filename").toString().isEmpty()) {
            documentFilenames.append(settings.value("filename").toString());
        }
    }
    settings.endArray();
    windows.first()->openDocuments(documentFilenames, false);

    settings.beginReadArray("windows");
    for (int i = 0; i < windowCount; ++i) {
        settings.setArrayIndex(i);
        MainWindow *const window = windows[i];
        window->readEditorSettings(settings);
    }
    settings.endArray();
}

void SessionManager::writeSession(QSettings &settings) const
{
    settings.remove("documents");
    settings.beginWriteArray("documents");
    for (int i = 0; i < qApp->documentManager.documentCount(); ++i) {
        settings.setArrayIndex(i);
        Scene *const document = qApp->documentManager.documents()[i];
        settings.setValue("filename", document->filename());
    }
    settings.endArray();

    settings.remove("windows");
    settings.beginWriteArray("windows");
    for (int i = 0; i < m_windows.count(); ++i) {
        settings.setArrayIndex(i);
        MainWindow *const window = m_windows[i];
        window->writeSettings(settings);
    }
    settings.endArray();
}

void SessionManager::updateWindowActivationOrder(MainWindow *const window)
{
    windowActivationOrder.removeOne(window);
    windowActivationOrder.append(window);
}

void SessionManager::closeSession()
{
    for (auto document :  qApp->documentManager.documents()) {
        qApp->documentManager.closeDocument(document);
    }
    for (auto window : m_windows) {
        deleteWindow(window);
    }
}

bool SessionManager::openSession(const QString &filename) {
    QSettings session(filename, QSettings::IniFormat);
    if ((session.status() != QSettings::NoError) || !session.childGroups().contains("windows")) return false;
    closeSession();
    m_sessionFilename = filename;
    readSession(session);
    return true;
}

bool SessionManager::saveSession(QString filename)
{
    QSettings session(filename, QSettings::IniFormat);
    if (session.status() != QSettings::NoError || !session.isWritable()) {
        return false;
    }
    else {
        m_sessionFilename = filename;
        writeSession(session);
        return true;
    }
}

void SessionManager::newSession()
{
    closeSession();
    m_sessionFilename = QString();
    newWindow();
}

MainWindow *SessionManager::newWindow()
{
    MainWindow *window = new MainWindow();
    m_windows.append(window);
    window->installEventFilter(this);
    window->show();
    return window;
}

void SessionManager::deleteWindow(MainWindow *const window)
{
    m_windows.removeOne(window);
    windowActivationOrder.removeAll(window);
    window->hide();
    window->deleteLater();
}

bool SessionManager::eventFilter(QObject *const watched, QEvent *const event)
{
    if (event->type() == QEvent::Close) {
        MainWindow *const window = qobject_cast<MainWindow *>(watched);
        if (window) {
            if ((m_windows.count() == 1 && window->promptExit()) ||
                    window->closePrompt()) {
                deleteWindow(window);
                event->accept();
            }
            else event->ignore();
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

} // namespace GfxPaint
