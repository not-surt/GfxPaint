#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QActionGroup>
#include <QList>

class QMdiSubWindow;
class QItemSelection;
class QSettings;

namespace GfxPaint {

class Application;
class Editor;
class Scene;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *const parent = nullptr, const Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindow() override;

    virtual bool eventFilter(QObject *const watched, QEvent *const event) override;

    void newSession();
    void openSession();
    bool saveSession(const bool saveAs = false);
    void saveSessionAs();
    bool promptCloseWindows(const QList<MainWindow *> &windows);
    bool promptExit();

    bool openDocument(const QString &filename, const bool createEditor);
    void openDocuments(const QStringList &filenames, const bool createEditors);

    bool promptSaveDocument(Scene *const document, const bool saveAs = false);

    bool promptCloseDocument(Scene *const document);
    bool closeDocument(Scene *const document);
    bool closeDocuments(const QList<Scene *> &documents);

    void readSettings(QSettings &settings);
    void readEditorSettings(QSettings &settings);
    void writeSettings(QSettings &settings) const;
    bool closePrompt(QSet<Scene *> *const closingDocuments = nullptr);

    void addEditor(Editor *const editor);
    void removeEditor(Editor *const editor);
    void moveEditor(MainWindow *const other, Editor *const editor);
    void deleteEditor(Editor *const editor);

public slots:
    void activateDocument(Scene *const document);
    void activateDocumentManagerIndex(const QModelIndex &index);
    void activateDocumentItemSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void activateEditor(Editor *const editor);
    void activateSubWindow(QMdiSubWindow *const subWindow);

    void openRecentSession();
    void openRecentFile();

    void newWindow();
    void closeWindow();
    void closeOtherWindows();

    void newFile();
    void revertFile();
    void openFiles();
    void saveFile(const bool saveAs = false);
    void saveFileAs();
    void closeFile();
    void closeOtherFiles();
    void closeAllFiles();

    void newEditor();
    void duplicateEditor();
    void closeEditor();
    void closeOtherEditors();
    void closeAllEditors();

protected:
    virtual bool event(QEvent *const event) override;

    void rebuildFileActionsMenu(QMenu *const menu, const QStringList &filenames, void (MainWindow::*slot)());
    void rebuildRecentSessionsMenu();
    void rebuildRecentFilesMenu();
    void buildNodesMenu();

    void filesViewContextMenu(const QPoint &pos);

    Ui::MainWindow *const ui;
    QActionGroup pixelRatiosGroup;
    QActionGroup toolsGroup;
    QActionGroup blendModesGroup;
    QActionGroup tabPositionsGroup;
    Scene *activeDocument;
    Editor *activeEditor;
    QList<QMetaObject::Connection> activeEditorConnections;
    QMap<Editor *, QMdiSubWindow *> editorSubWindows;
};

} // namespace GfxPaint

#endif // MAINWINDOW_H
