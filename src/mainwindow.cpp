#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGlobal>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QSettings>
#include <QProgressDialog>
#include <QFileDialog>
#include <QProgressBar>
#if defined(Q_OS_WIN) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    #include <QtPlatformHeaders\QWindowsWindowFunctions>
#endif

#include "newbufferdialog.h"
#include "application.h"
#include "types.h"
#include "multitoolbutton.h"
#include "utils.h"
#include "editor.h"

namespace GfxPaint {

MainWindow::MainWindow(QWidget *const parent, const Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      ui(new Ui::MainWindow),
    pixelRatiosGroup(this), toolGroup(this), menuToolSpace(nullptr), toolSpaceGroup(this), blendGroup(this), composeGroup(this), tabPositionsGroup(this),
    activeDocument(nullptr), activeEditor(nullptr), activeEditorConnections(), editorSubWindows()
{
#if defined(Q_OS_WIN) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    winId(); // Allocate window handle
    QWindowsWindowFunctions::setHasBorderInFullScreen(windowHandle(), true);
#endif

    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    QObject::connect(qApp, &Application::reopenSessionAtStartupChanged, ui->actionReopenSessionAtStartup, &QAction::setChecked);
    QObject::connect(ui->actionReopenSessionAtStartup, &QAction::toggled, qApp, &Application::setReopenSessionAtStartup);
    ui->actionReopenSessionAtStartup->setChecked(qApp->reopenSessionAtStartup());
    QObject::connect(qApp, &Application::saveSessionAtExitChanged, ui->actionSaveSessionAtExit, &QAction::setChecked);
    QObject::connect(ui->actionSaveSessionAtExit, &QAction::toggled, qApp, &Application::setSaveSessionAtExit);
    ui->actionSaveSessionAtExit->setChecked(qApp->saveSessionAtExit());

    ui->sessionEditorWidget->setModel(&qApp->documentManager);
    QObject::connect(&qApp->documentManager, &DocumentsModel::requestCloseDocument, this, &MainWindow::closeDocument);
    //QObject::connect(ui->filesView, &QAbstractItemView::activated, this, &MainWindow::activateDocumentManagerIndex);
    QObject::connect(ui->sessionEditorWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::activateDocumentItemSelection);
    //QObject::connect(ui->documentsView, &QTreeView::customContextMenuRequested, this, &MainWindow::filesViewContextMenu);

    rebuildRecentSessionsMenu();
    rebuildRecentFilesMenu();

    buildNodesMenu();

    ui->menuStyle->addActions(qApp->styleActions());
    ui->menuApplicationPalette->addActions(qApp->paletteActions());
    ui->menuStylesheet->addActions(qApp->stylesheetActions());

    for (auto action : ui->menuBar->actions()) {
        addAction(action);
    }

    for (auto toolBar : findChildren<QToolBar *>(QString(), Qt::FindDirectChildrenOnly)) {
        ui->menuToolbars->addAction(toolBar->toggleViewAction());
    }
    for (auto dock : findChildren<QDockWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        ui->menuDocks->addAction(dock->toggleViewAction());
    }

    QObject::connect(qApp, &Application::recentSessionsChanged, this, &MainWindow::rebuildRecentSessionsMenu);
    QObject::connect(qApp, &Application::recentFilesChanged, this, &MainWindow::rebuildRecentFilesMenu);
    QObject::connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::activateSubWindow);

    QObject::connect(ui->actionNewSession, &QAction::triggered, this, &MainWindow::newSession);
    QObject::connect(ui->actionOpenSession, &QAction::triggered, this, &MainWindow::openSession);
    QObject::connect(ui->actionSaveSession, &QAction::triggered, this, &MainWindow::saveSession);
    QObject::connect(ui->actionSaveSessionAs, &QAction::triggered, this, &MainWindow::saveSessionAs);
    QObject::connect(ui->actionAbout, &QAction::triggered, this, [this](){
        QMessageBox::about(this, QString("About %1").arg(qApp->applicationName()), "Well, what about it?");
    });
    QObject::connect(ui->actionExit, &QAction::triggered, this, &MainWindow::promptExit);

    QObject::connect(ui->actionNewWindow, &QAction::triggered, this, &MainWindow::newWindow);
    QObject::connect(ui->actionCloseWindow, &QAction::triggered, this, &MainWindow::closeWindow);
    QObject::connect(ui->actionCloseOtherWindows, &QAction::triggered, this, &MainWindow::closeOtherWindows);
    QObject::connect(ui->actionNewEditor, &QAction::triggered, this, &MainWindow::newEditor);
    QObject::connect(ui->actionDuplicateEditor, &QAction::triggered, this, &MainWindow::duplicateEditor);
    QObject::connect(ui->actionCloseEditor, &QAction::triggered, this, &MainWindow::closeEditor);
    QObject::connect(ui->actionCloseOtherEditors, &QAction::triggered, this, &MainWindow::closeOtherEditors);
    QObject::connect(ui->actionCloseAllEditors, &QAction::triggered, this, &MainWindow::closeAllEditors);
    QObject::connect(ui->actionCascadeEditors, &QAction::triggered, ui->mdiArea, &QMdiArea::cascadeSubWindows);
    QObject::connect(ui->actionTileEditors, &QAction::triggered, ui->mdiArea, &QMdiArea::tileSubWindows);
    QObject::connect(ui->actionTabbedEditors, &QAction::toggled, this, [this](bool checked){
        ui->mdiArea->setViewMode(checked ? QMdiArea::TabbedView : QMdiArea::SubWindowView);
    });
    ui->actionTabbedEditors->setChecked(ui->actionTabbedEditors->isChecked());

    const QList<QAction *> tabPositionsActions = {
        ui->actionTabPositionTop, ui->actionTabPositionBottom, ui->actionTabPositionLeft, ui->actionTabPositionRight
    };
    tabPositionsGroup.setExclusive(true);
    for (auto action : tabPositionsActions) tabPositionsGroup.addAction(action);
    ui->actionTabPositionTop->setData(QTabWidget::North);
    ui->actionTabPositionBottom->setData(QTabWidget::South);
    ui->actionTabPositionLeft->setData(QTabWidget::West);
    ui->actionTabPositionRight->setData(QTabWidget::East);
    QObject::connect(&tabPositionsGroup, &QActionGroup::triggered, this, [this](QAction *const action){
        ui->mdiArea->setTabPosition(static_cast<QTabWidget::TabPosition>(action->data().toInt()));
    });
    ui->actionTabPositionTop->setChecked(ui->actionTabPositionTop->isChecked());

    QObject::connect(ui->actionNewFile, &QAction::triggered, this, &MainWindow::newFile);
    QObject::connect(ui->actionRevertFile, &QAction::triggered, this, &MainWindow::revertFile);
    QObject::connect(ui->actionOpenFiles, &QAction::triggered, this, &MainWindow::openFiles);
    QObject::connect(ui->actionSaveFile, &QAction::triggered, this, &MainWindow::saveFile);
    QObject::connect(ui->actionSaveFileAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    QObject::connect(ui->actionCloseFile, &QAction::triggered, this, &MainWindow::closeFile);
    QObject::connect(ui->actionCloseOtherFiles, &QAction::triggered, this, &MainWindow::closeOtherFiles);
    QObject::connect(ui->actionCloseAllFiles, &QAction::triggered, this, &MainWindow::closeAllFiles);

    QObject::connect(ui->actionFullscreen, &QAction::toggled, this, [this](bool checked){
        setWindowState(windowState().setFlag(Qt::WindowFullScreen, checked));
    });
    ui->actionFullscreen->setChecked(windowState().testFlag(Qt::WindowFullScreen));
    QObject::connect(ui->actionShowMenuBar, &QAction::toggled, ui->menuBar, &QMenuBar::setVisible);
    QObject::connect(ui->actionShowStatusBar, &QAction::toggled, ui->statusBar, &QStatusBar::setVisible);

    const QList<QAction *> pixelRatiosActions = {
        ui->actionActualPixelRatio, ui->actionNearestIntegerPixelRatio, ui->actionSquarePixelRatio
    };
    pixelRatiosGroup.setExclusive(true);
    for (auto action : pixelRatiosActions) pixelRatiosGroup.addAction(action);
    ui->actionActualPixelRatio->setChecked(true);

    QToolButton *const mainMenuToolButton = new QToolButton();
    mainMenuToolButton->setText("Menu");
    mainMenuToolButton->setPopupMode(QToolButton::InstantPopup);
    mainMenuToolButton->setAutoRaise(true);
    QMenu *const mainMenu = new QMenu(this);
    for (auto menu : ui->menuBar->actions()) {
        mainMenu->addAction(menu);
    }
    mainMenuToolButton->setMenu(mainMenu);
    ui->mainToolBar->insertWidget(ui->mainToolBar->actions().at(0), centringWidget(mainMenuToolButton));

    menuToolSpace = new QMenu(this);
    menuToolSpace->setTitle("Tool Space");
    toolSpaceGroup.setExclusive(true);
    for (auto &[space, name] : EditingContext::toolSpaceNames) {
        QAction *const action = new QAction(this);
        action->setText(name);
        action->setData(static_cast<int>(space));
        action->setCheckable(true);
        toolSpaceGroup.addAction(action);
        menuToolSpace->addAction(action);
    }
}

MainWindow::~MainWindow()
{
    const auto keys = editorSubWindows.keys();
    for (auto editor : keys) {
//        deleteEditor(editor); // If parented then should be deleted automatically?
    }
    delete ui;
}

void MainWindow::readSettings(QSettings &settings)
{
    if (settings.contains("geometry")) restoreGeometry(settings.value("geometry").toByteArray());
    ui->actionFullscreen->setChecked(windowState().testFlag(Qt::WindowFullScreen));
    if (settings.contains("state")) restoreState(settings.value("state").toByteArray());

    if (settings.contains("showMenuBar")) ui->actionShowMenuBar->setChecked(settings.value("showMenuBar").toBool());
    if (settings.contains("showStatusBar")) ui->actionShowStatusBar->setChecked(settings.value("showStatusBar").toBool());

    if (settings.contains("tabbedEditors")) ui->actionTabbedEditors->setChecked(settings.value("tabbedEditors").toBool());
    if (settings.contains("tabPosition")) {
        const int index = settings.value("tabPosition").toInt();
        if (index >= 0 && index < tabPositionsGroup.actions().size()) tabPositionsGroup.actions().at(index)->trigger();
    }

    if (settings.contains("createEditorWithFile")) ui->actionCreateEditorWithFile->setChecked(settings.value("createEditorWithFile").toBool());
    if (settings.contains("closeFileWithLastEditor")) ui->actionCloseFileWithLastEditor->setChecked(settings.value("closeFileWithLastEditor").toBool());

    if (settings.contains("tool")) {
        const int index = settings.value("tool").toInt();
        if (index >= 0 && index < toolGroup.actions().size()) toolGroup.actions().at(index)->trigger();
    }
}

void MainWindow::readEditorSettings(QSettings &settings)
{
    // Disable tabbed view to get correct subwindow geometries
    const bool tabbed = ui->actionTabbedEditors->isChecked();
    ui->actionTabbedEditors->setChecked(false);
    const int subWindowCount = settings.beginReadArray("subWindows");
    for (int i = 0; i < subWindowCount; ++i) {
        settings.setArrayIndex(i);
        if (settings.contains("document")) {
            const int index = settings.value("document").toInt();
            if (index >= 0 && index < qApp->documentManager.documentCount()) {
                Scene *const document = qApp->documentManager.documents()[index];
                Editor *editor = new Editor(*document);
                addEditor(editor);
                QMdiSubWindow *const subWindow = editorSubWindows[editor];

                Qt::WindowStates state = static_cast<Qt::WindowStates>(settings.value("state", Qt::WindowNoState).toInt());
                // Must set active state before geometry
                subWindow->setWindowState(subWindow->windowState().setFlag(Qt::WindowActive, state.testFlag(Qt::WindowActive)));
                // Must set geometry before sized states
                if (settings.contains("geometry")) subWindow->setGeometry(settings.value("geometry").toRect());
                subWindow->setWindowState(subWindow->windowState() | (state & ~Qt::WindowActive));
            }
        }
    }
    settings.endArray();
    ui->actionTabbedEditors->setChecked(tabbed);
}

void MainWindow::writeSettings(QSettings &settings) const
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());

    settings.setValue("showMenuBar", ui->actionShowMenuBar->isChecked());
    settings.setValue("showStatusBar", ui->actionShowStatusBar->isChecked());

    settings.remove("subWindows");
    settings.beginWriteArray("subWindows");

    for (int i = 0; i < ui->mdiArea->subWindowList().count(); ++i) {
        QMdiSubWindow *const subWindow = ui->mdiArea->subWindowList().at(i);
        settings.setArrayIndex(i);
        Editor *const editor = dynamic_cast<Editor *>(subWindow->widget());
        if (editor && !editor->scene.filename().isEmpty()) {
            settings.setValue("document", qApp->documentManager.documents().indexOf(&editor->scene));

            settings.setValue("state", static_cast<int>(subWindow->windowState()));
            // Ugly hack to work around broken normalGeometry
            QRect actualNormalGeometry;
            if (subWindow->windowState() & ~(Qt::WindowNoState | Qt::WindowActive)) {
                Qt::WindowStates state = subWindow->windowState();
                subWindow->setWindowState(Qt::WindowNoState);
                actualNormalGeometry = subWindow->normalGeometry();
                subWindow->setWindowState(state);
            }
            else actualNormalGeometry = subWindow->normalGeometry();
            settings.setValue("geometry", actualNormalGeometry);
        }
    }
    settings.endArray();
    settings.setValue("tabbedEditors", ui->actionTabbedEditors->isChecked());
    settings.setValue("tabPosition", tabPositionsGroup.actions().indexOf(tabPositionsGroup.checkedAction()));

    settings.setValue("createEditorWithFile", ui->actionCreateEditorWithFile->isChecked());
    settings.setValue("closeFileWithLastEditor", ui->actionCloseFileWithLastEditor->isChecked());

    settings.setValue("tool", toolGroup.actions().indexOf(toolGroup.checkedAction()));
}

void MainWindow::newSession()
{
    qApp->sessionManager.newSession();
}

void MainWindow::openSession()
{
    QSettings settings;
    const QString path = !qApp->sessionManager.sessionFilename().isEmpty() ? qApp->sessionManager.sessionFilename() : settings.value("file/openPath", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
    const QString filename = QFileDialog::getOpenFileName(nullptr, "Open Session", path, QString("%1 Session (*.%2)").arg(qApp->applicationName(), Application::sessionExtension));
    if (!filename.isEmpty()) {
        if (!qApp->sessionManager.openSession(filename)) {
            QMessageBox::critical(this, "Session Open Error", QString("Error opening session: %1").arg(filename));
        }
        else {
            qApp->updateRecentSessions(filename);
            settings.setValue("file/openPath", QFileInfo(filename).path());
        }
    }
}

bool MainWindow::promptCloseWindows(const QList<MainWindow *> &windows)
{
    QSet<Scene *> closingDocuments;
    for (auto window : windows) {
        qApp->setActiveWindow(window);
        if (!window->closePrompt(&closingDocuments)) {
            qApp->setActiveWindow(this);
            return false;
        }
    }
    qApp->setActiveWindow(this);
    for (auto document : qApp->documentManager.documents()) {
        if (!closingDocuments.contains(document)) {
            if (!promptCloseDocument(document)) {
                return false;
            }
        }
    }
    return true;
}

bool MainWindow::promptExit()
{
    if (promptCloseWindows(qApp->sessionManager.windows())) {
        qApp->exit();
        return true;
    }
    else return false;
}

bool MainWindow::saveSession(const bool saveAs)
{
    QSettings settings;
    QString path = !qApp->sessionManager.sessionFilename().isEmpty() ? qApp->sessionManager.sessionFilename() : settings.value("file/savePath", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
    QString filename = !(saveAs || qApp->sessionManager.sessionFilename().isEmpty()) ? qApp->sessionManager.sessionFilename() : QFileDialog::getSaveFileName(Application::activeWindow(), "Save Session", path, QString("%1 Session (*.%2)").arg(qApp->applicationName(), Application::sessionExtension));
    if (!filename.isEmpty()) {
        QFileInfo info(filename);
        if (info.suffix().isEmpty()) {
            filename = info.path() + "/" + info.baseName() + "." + Application::sessionExtension;
        }
        qApp->updateRecentSessions(filename);
        settings.setValue("file/openPath", QFileInfo(filename).path());
        if (!qApp->sessionManager.saveSession(filename)) {
            QMessageBox::critical(this, "Session Save Error", QString("Error saving session: %1").arg(filename));
            return false;
        }
        else return true;
    }
    else return false;
}

void MainWindow::saveSessionAs()
{
    saveSession(true);
}

bool MainWindow::closePrompt(QSet<Scene *> *const closingDocuments)
{
    QSet<Scene *> documents;
    QList<QMdiSubWindow *> promptMdiSubWindow;
    QList<QMdiSubWindow *> subWindows = ui->mdiArea->subWindowList(QMdiArea::ActivationHistoryOrder);
    std::reverse(subWindows.begin(), subWindows.end());
    for (auto subWindow : subWindows) {
        Editor *const editor = dynamic_cast<Editor *>(subWindow->widget());
        Scene *const document = &editor->scene;
        if (!documents.contains(document) && document->modified() && qApp->documentManager.documentEditorCount(document) == 1) {
            promptMdiSubWindow.append(subWindow);
            documents.insert(document);
        }
    }
    QMdiSubWindow *const activeSubWindow = ui->mdiArea->activeSubWindow();
    for (auto subWindow : promptMdiSubWindow) {
        Editor *const editor = dynamic_cast<Editor *>(subWindow->widget());
        ui->mdiArea->setActiveSubWindow(subWindow);
        if (ui->actionCloseFileWithLastEditor->isChecked() && qApp->documentManager.documentEditorCount(&editor->scene) == 1) {
            if (!promptCloseDocument(&editor->scene)) {
                ui->mdiArea->setActiveSubWindow(activeSubWindow);
                return false;
            }
        }
    }
    if(closingDocuments) closingDocuments->unite(documents);
    return true;
}

bool MainWindow::event(QEvent *const event)
{
    if (event->type() == QEvent::WindowActivate) {
        qApp->sessionManager.updateWindowActivationOrder(this);
    }
    return QMainWindow::event(event);
}

bool MainWindow::openDocument(const QString &filename, const bool createEditor)
{
    Scene *const document = qApp->documentManager.openDocument(filename);
    if (!document) {
        QMessageBox::critical(this, "File Open Error", QString("Error opening file: %1").arg(filename));
        return false;
    }
    else {
        QSettings settings;
        settings.setValue("file/openPath", QFileInfo(filename).path());
        qApp->updateRecentFiles(filename);
        if (createEditor) {
            Editor *const editor = new Editor(*document);
            addEditor(editor);
        }
        return true;
    }
}

void MainWindow::openDocuments(const QStringList &filenames, const bool createEditors)
{

    QProgressDialog progress(this);
    QProgressBar *bar = new QProgressBar();
    bar->setFormat("%v of %m");
    progress.setBar(bar);
    progress.setLabelText("Opening files...");
    progress.setMinimum(0);
    progress.setMaximum(filenames.length() - 1);
    progress.setMinimumDuration(0);
    progress.setWindowModality(Qt::ApplicationModal);

    for (int i = 0; i < filenames.count(); i++) {
        progress.setValue(i);
        if (progress.wasCanceled()) break;
        else {
            const QString &filename = filenames[i];
            openDocument(filename, createEditors);
        }
    }
}

//void DocumentManager::saveDocuments(MainWindow *const window, const QList<Scene *> &documents)
//{
//    QProgressDialog progress(window);
//    progress.setLabelText("Saving files...");
//    progress.setMinimum(0);
//    progress.setMaximum(documents.count() - 1);
//    progress.setMinimumDuration(0);
//    progress.setWindowModality(Qt::ApplicationModal);

//    for (int i = 0; i < documents.count(); i++) {
//        progress.setValue(i);
//        if (progress.wasCanceled()) break;
//        else {
//            Scene *const document = documents[i];
//        }
//    }
//}

//void DocumentManager::saveAllDocuments(MainWindow *const window)
//{
//    saveDocuments(window, documents());
//}

bool MainWindow::promptCloseDocument(Scene *const document)
{
    if (document->modified()) {
        QMessageBox::StandardButton button = QMessageBox::question(this, "Unsaved Changes", QString("File \"%1\" has unsaved changes.").arg(document->filename()), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
        if ((button == QMessageBox::Save && promptSaveDocument(document)) || button == QMessageBox::Discard) return true;
        else return false;
    }
    else return true;
}

bool MainWindow::closeDocument(Scene *const document)
{
    if (promptCloseDocument(document)) {
        for (auto editor : qApp->documentManager.documentEditors(document)) {
            deleteEditor(editor);
        }
        qApp->documentManager.removeDocument(document);
        delete document;
        return true;
    }
    else return false;
}

bool MainWindow::closeDocuments(const QList<Scene *> &documents)
{
//    bool allClosed = true;
//    for (auto document : documents) {
//        allClosed = allClosed && closeDocument(document);
//    }
//    return allClosed;
    return std::all_of(documents.begin(), documents.end(), [this](Scene *document){ return closeDocument(document); });
}

void MainWindow::addEditor(Editor *const editor)
{
    qApp->documentManager.addEditor(editor);
    QMdiSubWindow *const subWindow = new QMdiSubWindow();
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->installEventFilter(this);
    editorSubWindows.insert(editor, subWindow);
    editor->setDocumentFilename(editor->scene.filename());
    subWindow->setWidget(editor);
    ui->mdiArea->addSubWindow(subWindow);
    subWindow->show();
}

void MainWindow::removeEditor(Editor *const editor)
{
    Q_ASSERT(editorSubWindows.contains(editor));

    QMdiSubWindow *const subWindow = editorSubWindows[editor];
    editorSubWindows.remove(editor);
    ui->mdiArea->removeSubWindow(subWindow);
    subWindow->deleteLater();
    editor->setParent(nullptr);
    qApp->documentManager.removeEditor(editor);
}

void MainWindow::moveEditor(MainWindow *const other, Editor *const editor)
{
    this->removeEditor(editor);
    other->addEditor(editor);
}

void MainWindow::deleteEditor(Editor *const editor)
{
    removeEditor(editor);
    editor->hide();
    editor->deleteLater();
}

void MainWindow::activateDocument(Scene *const document)
{
    activeDocument = document;
    if (!activeEditor || &activeEditor->scene != activeDocument) {
        for (auto subWindow : ui->mdiArea->subWindowList(QMdiArea::ActivationHistoryOrder)) {
            Editor *const editor = dynamic_cast<Editor *>(subWindow->widget());
            if (&editor->scene == activeDocument) {
                activateSubWindow(subWindow);
                break;
            }
        }
    }
}

void MainWindow::activateDocumentManagerIndex(const QModelIndex &index)
{
    Scene *document = qApp->documentManager.documents().at(index.row());
    activateDocument(document);
}

void MainWindow::activateDocumentItemSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.indexes().count() > 0) activateDocumentManagerIndex(selected.indexes()[0]);
}

void MainWindow::activateEditor(Editor *const editor)
{
    for (auto &connection : activeEditorConnections) {
        QObject::disconnect(connection);
    }
    activeEditorConnections.clear();
    removeEventFilter(activeEditor);
    for (auto action : toolGroup.actions()) {
        toolGroup.removeAction(action);
    }
    auto actions = ui->toolToolBar->actions();
    ui->menuTool->clear();
    ui->toolToolBar->clear();
    ui->menuBlend->clear();
    ui->menuCompose->clear();
    ui->undoView->setStack(nullptr);
    qDeleteAll(actions);
    activeEditor = editor;
    if (activeEditor) {
        ui->sessionEditorWidget->selectionModel()->select(qApp->documentManager.documentModelIndex(&activeEditor->scene), QItemSelectionModel::ClearAndSelect);

        activeEditorConnections << QObject::connect(ui->dabEditorWidget, &DabEditorWidget::dabChanged, this, [this](const Brush::Dab &dab){
            Brush brush = activeEditor->editingContext().brush();
            brush.dab = dab;
            activeEditor->setBrush(brush);
        });
        activeEditorConnections << QObject::connect(ui->strokeEditorWidget, &StrokeEditorWidget::strokeChanged, this, [this](const Brush::Stroke &stroke){
            Brush brush = activeEditor->editingContext().brush();
            brush.stroke = stroke;
            activeEditor->setBrush(brush);
        });
        activeEditorConnections << QObject::connect(activeEditor, &Editor::brushChanged, this, [this](const Brush &brush){
            activeEditor->editingContext().setBrush(brush);
            ui->dabEditorWidget->setDab(brush.dab);
            ui->strokeEditorWidget->setStroke(brush.stroke);
        });
        activeEditorConnections << QObject::connect(activeEditor, &Editor::brushChanged, ui->brushViewWidget, &BrushViewWidget::setBrush);

        activeEditorConnections << QObject::connect(activeEditor, &Editor::colourChanged, ui->colourSlidersWidget, &ColourSlidersWidget::setColour);
        activeEditorConnections << QObject::connect(ui->colourSlidersWidget, &ColourSlidersWidget::colourChanged, activeEditor, &Editor::setColour);
        activeEditorConnections << QObject::connect(activeEditor, &Editor::colourChanged, ui->colourPlaneWidget, &ColourPlaneWidget::setColour);
        activeEditorConnections << QObject::connect(ui->colourPlaneWidget, &ColourPlaneWidget::colourChanged, activeEditor, &Editor::setColour);
        activeEditorConnections << QObject::connect(ui->paletteEditorWidget, &PaletteEditorWidget::colourPicked, activeEditor, &Editor::setColour);
        activeEditorConnections << QObject::connect(activeEditor, &Editor::colourChanged, ui->brushViewWidget, &BrushViewWidget::setColour);
        activeEditorConnections << QObject::connect(activeEditor, &Editor::paletteChanged, ui->colourSlidersWidget, &ColourSlidersWidget::setPalette);
        activeEditorConnections << QObject::connect(activeEditor, &Editor::paletteChanged, ui->colourPlaneWidget, &ColourPlaneWidget::setPalette);
        activeEditorConnections << QObject::connect(activeEditor, &Editor::paletteChanged, ui->paletteEditorWidget, &PaletteEditorWidget::setPalette);

        ui->sceneTreeWidget->setEnabled(true);
        ui->sceneTreeWidget->setEditor(editor);
        activeEditorConnections << QObject::connect(editor, &QObject::destroyed, this, [this](QObject *const object){ui->sceneTreeWidget->setEditor(nullptr);});

        activeEditorConnections << QObject::connect(activeEditor, &Editor::transformChanged, ui->transformEditorWidget, &TransformEditorWidget::setTransform);
        activeEditorConnections << QObject::connect(ui->transformEditorWidget, &TransformEditorWidget::transformChanged, activeEditor, &Editor::setTransform);
        activeEditorConnections << QObject::connect(activeEditor, &Editor::transformTargetChanged, ui->transformEditorWidget, &TransformEditorWidget::setTransformTarget);
        activeEditorConnections << QObject::connect(ui->transformEditorWidget, &TransformEditorWidget::transformTargetChanged, activeEditor, &Editor::setTransformTarget);

        QUndoStack *const undoStack = qApp->documentManager.documentUndoStack(&activeEditor->scene);
        ui->undoView->setStack(undoStack);
        auto menuEditActions = ui->menuEdit->actions();
        ui->menuEdit->clear();
        qDeleteAll(menuEditActions);
        ui->menuEdit->addAction(undoStack->createUndoAction(this));
        ui->menuEdit->addAction(undoStack->createRedoAction(this));

        toolGroup.setExclusive(true);
        ui->menuTool->addMenu(menuToolSpace);
        toolIdToAction.clear();
        actionToToolId.clear();
        for (const auto &group : activeEditor->toolMenuGroups) {
            MultiToolButton *const toolButton = new MultiToolButton();
            QAction *defaultAction = nullptr;
            ui->menuTool->addSeparator();
            for (const Editor::ToolId id : group.first) {
                const Editor::ToolInfo &info = activeEditor->toolInfo.at(id);
                QAction *action = new QAction();
                action->setText(info.name);
                action->setCheckable(true);
                action->setData(static_cast<int>(id));
                ui->menuTool->addAction(action);
                toolButton->addAction(action);
                toolGroup.addAction(action);
                if (id == group.second) {
                    defaultAction = action;
                }
                if (id == activeEditor->defaultTool) {
                    action->setChecked(true);
                }
                toolIdToAction.emplace(id, action);
                actionToToolId.emplace(action, id);
            }
            toolButton->setDefaultAction(defaultAction);
            ui->toolToolBar->addWidget(centringWidget(toolButton));
        }
        activeEditorConnections << QObject::connect(activeEditor, &Editor::selectedToolIdChanged, this, [this](const Editor::ToolId toolId){
            toolIdToAction.at(toolId)->setChecked(true);
        });
        activeEditorConnections << QObject::connect(&toolGroup, &QActionGroup::triggered, this, [this](QAction *const action){
            activeEditor->setSelectedToolId(actionToToolId.at(action));
        });
        activeEditorConnections << QObject::connect(activeEditor, &Editor::toolSpaceChanged, this, [this](const EditingContext::ToolSpace toolSpace){
            toolSpaceGroup.actions()[static_cast<int>(toolSpace)]->setChecked(true);
        });
        activeEditorConnections << QObject::connect(&toolSpaceGroup, &QActionGroup::triggered, this, [this](QAction *const action){
            activeEditor->editingContext().setSpace(static_cast<EditingContext::ToolSpace>(action->data().toInt()));
        });
        const int space = static_cast<int>(activeEditor->editingContext().space());
        if (space >= 0 && space < toolSpaceGroup.actions().size()) {
            toolSpaceGroup.actions()[space]->setChecked(true);
        }

        blendGroup.setExclusive(true);
        for (int i = 0; i < RenderManager::blendModes.size(); ++i) {
            auto &[name, func] = RenderManager::blendModes[i];
            QAction *action = new QAction();
            action->setText(name);
            action->setCheckable(true);
            action->setData(i);
            ui->menuBlend->addAction(action);
            blendGroup.addAction(action);
        }
        activeEditorConnections << QObject::connect(activeEditor, &Editor::blendModeChanged, this, [this](const int blendMode){
            Q_ASSERT(blendMode >= 0 && blendMode < blendGroup.actions().size());
            blendGroup.actions()[blendMode]->setChecked(true);
        });
        activeEditorConnections << QObject::connect(&blendGroup, &QActionGroup::triggered, this, [this](QAction *const action){
            activeEditor->setBlendMode(action->data().toInt());
        });

        composeGroup.setExclusive(true);
        for (int i = 0; i < RenderManager::composeModes.size(); ++i) {
            auto &[name, func] = RenderManager::composeModes[i];
            QAction *action = new QAction();
            action->setText(name);
            action->setCheckable(true);
            action->setData(i);
            ui->menuCompose->addAction(action);
            composeGroup.addAction(action);
        }
        activeEditorConnections << QObject::connect(activeEditor, &Editor::composeModeChanged, this, [this](const int composeMode){
            Q_ASSERT(composeMode >= 0 && composeMode < blendGroup.actions().size());
            composeGroup.actions()[composeMode]->setChecked(true);
        });
        activeEditorConnections << QObject::connect(&composeGroup, &QActionGroup::triggered, this, [this](QAction *const action){
            activeEditor->setComposeMode(action->data().toInt());
        });

        installEventFilter(activeEditor);

        activeEditor->activeEditingContextUpdated();
    }
    else {
        ui->sceneTreeWidget->setEditor(nullptr);
        ui->sceneTreeWidget->setEnabled(false);
    }
}

void MainWindow::activateSubWindow(QMdiSubWindow *const subWindow)
{
    if (subWindow) {
        if (ui->mdiArea->activeSubWindow() != subWindow) ui->mdiArea->setActiveSubWindow(subWindow);
        Editor *const editor = dynamic_cast<Editor *>(subWindow->widget());
        activateEditor(editor);
    }
    else activateEditor(nullptr);
}

void MainWindow::rebuildFileActionsMenu(QMenu *const menu, const QStringList &filenames, void (MainWindow::*slot)())
{
    menu->clear();
    for (int i = 0; i < filenames.length(); ++i) {
        const QString &filename = filenames.at(i);
        QAction *const action = new QAction(QFileInfo(filename).fileName(), this);
        action->setData(filename);
        QObject::connect(action, &QAction::triggered, this, slot);
        menu->addAction(action);
    }
}

void MainWindow::openRecentSession()
{
    QAction *const action = qobject_cast<QAction *>(sender());
    if (action) qApp->sessionManager.openSession(action->data().toString());
}

void MainWindow::openRecentFile()
{
    QAction *const action = qobject_cast<QAction *>(sender());
    if (action) openDocument(action->data().toString(), true);
}

void MainWindow::newWindow()
{
    qApp->sessionManager.newWindow();
}

void MainWindow::closeWindow()
{
    this->close();
}

void MainWindow::closeOtherWindows()
{
    QList<MainWindow *> otherWindows = qApp->sessionManager.windows();
    otherWindows.removeOne(this);
    if (promptCloseWindows(otherWindows)) {
        qDeleteAll(otherWindows);
    }
}

void MainWindow::rebuildRecentSessionsMenu()
{
    rebuildFileActionsMenu(ui->menuRecentSessions, qApp->recentSessions(), &MainWindow::openRecentSession);
}

void MainWindow::rebuildRecentFilesMenu()
{
    rebuildFileActionsMenu(ui->menuRecentFiles, qApp->recentFiles(), &MainWindow::openRecentFile);
}

void MainWindow::buildNodesMenu()
{
    ui->menuNode->clear();
    for (int i = 0; i < Application::nodeInfo.length(); ++i) {
        const Application::NodeInfo &nodeInfo = Application::nodeInfo[i];
        if (nodeInfo.create) {
            QAction *const action = new QAction("Add " + nodeInfo.label, this);
            ui->menuNode->addAction(action);
            QObject::connect(action, &QAction::triggered, this, [this, nodeInfo](){
                if (activeEditor) {
                    Node *const node = nodeInfo.create();
                    activeEditor->insertNodes({node});
                }
            });
        }
        if (nodeInfo.createFromDialog) {
            QAction *const action = new QAction("Add " + nodeInfo.label + "...", this);
            ui->menuNode->addAction(action);
            QObject::connect(action, &QAction::triggered, this, [this, nodeInfo](){
                if (activeEditor) {
                    Node *const node = nodeInfo.createFromDialog(this);
                    if (node) activeEditor->insertNodes({node});
                }
            });
        }
        if (nodeInfo.createFromFile) {
            QAction *const action = new QAction("Add " + nodeInfo.label + " from Files...", this);
            ui->menuNode->addAction(action);
            QObject::connect(action, &QAction::triggered, this, [this, nodeInfo](){
                if (activeEditor) {
                    QSettings settings;
                    const QString path = settings.value("file/openPath", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
                    const QStringList filenames = QFileDialog::getOpenFileNames(this, "Open File", path, Application::openImageFilters());
                    if (!filenames.isEmpty()) {
                        QList<Node *> nodes;
                        for (const auto &filename : filenames) {
                            nodes.append(nodeInfo.createFromFile(filename));
                        }
                        activeEditor->insertNodes(nodes);
                        settings.setValue("file/openPath", QFileInfo(filenames.last()).path());
                    }
                }
            });
        }
    }
    ui->menuNode->addSeparator();
    QAction *const removeNodesAction = new QAction("Remove nodes", this);
    ui->menuNode->addAction(removeNodesAction);
    QObject::connect(removeNodesAction, &QAction::triggered, this, [this](){
        if (activeEditor) {
            activeEditor->removeSelectedNodes();
        }
    });
    QAction *const duplicateNodesAction = new QAction("Duplicate nodes", this);
    ui->menuNode->addAction(duplicateNodesAction);
    QObject::connect(removeNodesAction, &QAction::triggered, this, [this](){
        if (activeEditor) {
            activeEditor->duplicateSelectedNodes();
        }
    });
}

void MainWindow::filesViewContextMenu(const QPoint &pos)
{
//    const QPoint globalPos = ui->documentsView->mapToGlobal(pos);
//    const QModelIndex index = ui->documentsView->indexAt(pos);
//    if (index.isValid()) {
//        ui->menuFile->exec(globalPos);
//    }
//    else {

//    }
}

bool MainWindow::promptSaveDocument(Scene *const document, const bool saveAs)
{
    QSettings settings;
    if (document) {
        QString path = !document->filename().isEmpty() ? document->filename() : settings.value("file/savePath", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
        QString filename = document->filename();
        if (filename.isEmpty() || saveAs) {
            filename = QFileDialog::getSaveFileName(this, "Save Image", path, Application::saveImageFilters());
        }
        if (!filename.isEmpty()) {
            QFileInfo info(filename);
            if (info.suffix().isEmpty()) {
                filename = info.path() + "/" + info.baseName() + ".png";
            }
            qApp->documentManager.saveDocument(document, filename);
            qApp->updateRecentFiles(filename);
            if (saveAs) settings.setValue("file/savePath", QFileInfo(filename).path());
            return true;
        }
    }
    return false;
}

void MainWindow::newFile()
{
    Scene *const document = qApp->documentManager.newDocument();
    if (ui->actionCreateEditorWithFile->isChecked()) {
        Editor *const editor = new Editor(*document);
        addEditor(editor);
    }
}

void MainWindow::revertFile()
{
}

void MainWindow::openFiles()
{
    QSettings settings;
    const QString path = settings.value("file/openPath", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
    const QStringList filenames = QFileDialog::getOpenFileNames(this, "Open File", path, Application::openImageFilters());

    if (!filenames.isEmpty()) {
        openDocuments(filenames, ui->actionCreateEditorWithFile->isChecked());
        settings.setValue("file/openPath", QFileInfo(filenames.last()).path());
    }
}

void MainWindow::saveFile(const bool saveAs)
{
    if (activeDocument) promptSaveDocument(activeDocument, saveAs);
}

void MainWindow::saveFileAs()
{
    saveFile(true);
}

void MainWindow::closeFile()
{
    if (activeDocument) closeDocument(activeDocument);
}

void MainWindow::closeOtherFiles()
{
    if (activeDocument) {
        QList<Scene *> documents = qApp->documentManager.documents();
        documents.removeOne(activeDocument);
        closeDocuments(documents);
    }
}

void MainWindow::closeAllFiles()
{
    closeDocuments(qApp->documentManager.documents());
}

void MainWindow::newEditor()
{
    if (activeDocument) {
        Editor *const editor = new Editor(*activeDocument);
        addEditor(editor);
    }
}

void MainWindow::duplicateEditor()
{
    if (activeEditor) {
        Editor *const editor = new Editor(*activeEditor);
        addEditor(editor);
    }
}

void MainWindow::closeEditor()
{
    if (activeEditor) editorSubWindows[activeEditor]->close();
}

void MainWindow::closeOtherEditors()
{
    QList<QMdiSubWindow *> mdiSubWindows = ui->mdiArea->subWindowList();
    for (auto subWindow : mdiSubWindows) {
        if (subWindow != ui->mdiArea->activeSubWindow()) subWindow->close();
    }
}

void MainWindow::closeAllEditors()
{
    ui->mdiArea->closeAllSubWindows();
}

bool MainWindow::eventFilter(QObject *const watched, QEvent *const event)
{
    if (event->type() == QEvent::Close) {
        QMdiSubWindow *const subWindow = qobject_cast<QMdiSubWindow *>(watched);
        if (subWindow) {
            Editor *const editor = dynamic_cast<Editor *>(subWindow->widget());
            Scene *const document = &editor->scene;
            if (ui->actionCloseFileWithLastEditor->isChecked() && qApp->documentManager.documentEditorCount(document) == 1) {
                if (closeDocument(document)) event->accept();
                else event->ignore();
            }
            else {
                deleteEditor(editor);
                event->accept();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

} // namespace GfxPaint
