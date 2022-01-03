#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

#include <QOffscreenSurface>
#include <QMenu>
#include <QActionGroup>
#include <QElapsedTimer>

#include "types.h"
#include "documentmanager.h"
#include "workbuffermanager.h"
#include "sessionmanager.h"
#include "rendermanager.h"

class QSettings;

namespace GfxPaint {

class MainWindow;
class Document;
class Editor;
class Image;
class Node;

class Application : public QApplication
{
    Q_OBJECT

public:
    enum class IconFont {
        Menu = 0xf0c9,
        Settings = 0xf013,

        FileNew = 0xf15b,
        FileOpen = 0xf07c,
        FileSave = 0xf0c7,
        FileClose = 0xf00d,

        EditUndo = 0xf0e2,
        EditRedo = 0xf01e,
        EditCut = 0xf0c4,
        EditCopy = 0xf0c5,
        EditPaste = 0xf0ea,

        ToolPixel = 0xf303,
        ToolBrush = 0xf1fc,
        ToolSpray = 0xf5bd,
        ToolEraser = 0xf12d,
        ToolRect = 0xf0c8,
        ToolEllipse = 0xf111,
        ToolFill = 0xf5aa,
        ToolReplace = 0xf53f,
        ToolPick = 0xf1fb,
        ToolSelect = 0xf065,
        ToolGrab = 0xf0c4,
    };

    explicit Application(int &argc, char **argv);
    virtual ~Application();

    void exit(const int returnCode = 0);

    void setGitRevision(const QString &gitRevision) { m_gitRevision = gitRevision; }
    const QString &gitRevision() { return m_gitRevision; }
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
    float time() const;

    static const std::map<QImage::Format, QImage::Format> qImageConversion;
    static const std::map<QImage::Format, Buffer::Format> qImageToBuffer;
    static const std::map<Buffer::Format, QImage::Format> bufferToQImage;
    static const QString sessionExtension;

    RenderManager renderManager;
    WorkBufferManager workBufferManager;
    SessionManager sessionManager;
    DocumentManager documentManager;

    const QFont &iconFont() const { return m_iconFont; }

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
    QString m_gitRevision;
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
    QElapsedTimer timer;
    QFont m_iconFont;
};

#ifdef qApp
#undef qApp
#endif
#define qApp (static_cast<Application *>(QCoreApplication::instance()))

} // namespace GfxPaint

#endif // APPLICATION_H
