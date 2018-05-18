#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include "opengl.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QMenu>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "types.h"
#include "documentmanager.h"
#include "workbuffermanager.h"
#include "sessionmanager.h"
#include "rendermanager.h"

class QSettings;
class QOpenGLShader;
class QOpenGLShaderProgram;

namespace GfxPaint {

class MainWindow;
class Document;
class Editor;
class Image;
class Node;

class Application : public QApplication, protected OpenGL
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    virtual ~Application();

    void exit(const int returnCode = 0);

    void updateRecentSessions(const QStringList &filenames = QStringList());
    void updateRecentSessions(const QString &filename);
    const QStringList &recentSessions() const;
    static QString openImageFilters();
    static QString saveImageFilters();
    void updateRecentFiles(const QStringList &filenames = QStringList());
    void updateRecentFiles(const QString &filename);
    const QStringList &recentFiles() const;
    QList<QAction *> styleActions();
    QList<QAction *> paletteActions();
    QList<QAction *> stylesheetActions();
    void readSettings(QSettings &settings);
    void writeSettings(QSettings &settings);
    bool reopenSessionAtStartup() const;
    bool saveSessionAtExit() const;

    static const QMap<QImage::Format, QImage::Format> qImageConversion;
    static const QMap<QImage::Format, Buffer::Format> qImageToBuffer;
    static const QMap<Buffer::Format, QImage::Format> bufferToQImage;
    static const QString sessionExtension;

    RenderManager renderManager;
    WorkBufferManager workBufferManager;
    SessionManager sessionManager;
    DocumentManager documentManager;

    struct NodeInfo {
        QString label;
        Node *(*create)();
        Node *(*createFromFile)(const QString &filename);
        Node *(*createFromDialog)(QWidget *const parentWindow);
    };
    static const QList<NodeInfo> nodeInfo;

public slots:
    void commitData(QSessionManager &manager);
    void saveState(QSessionManager &manager);

    void setReopenSessionAtStartup(bool value);
    void setSaveSessionAtExit(bool value);

signals:
    void recentFilesChanged();
    void recentSessionsChanged();
    void reopenSessionAtStartupChanged(const bool value);
    void saveSessionAtExitChanged(const bool value);

protected:
    QStringList m_recentSessions;
    QStringList m_recentFiles;
    QMap<QString, QStyle *> styles;
    QActionGroup m_styleActions;
    QMap<QString, QPalette> palettes;
    QActionGroup m_paletteActions;
    QMap<QString, QString> stylesheets;
    QActionGroup m_stylesheetActions;
    bool m_reopenSessionAtStartup;
    bool m_saveSessionAtExit;
};

#ifdef qApp
#undef qApp
#endif
#define qApp (static_cast<Application *>(QCoreApplication::instance()))

} // namespace GfxPaint

#endif // APPLICATION_H
