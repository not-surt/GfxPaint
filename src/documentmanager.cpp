#include "documentmanager.h"

#include <QFileInfo>
#include <QWidget>
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QProgressBar>
#include <QMdiSubWindow>

#include "mainwindow.h"
#include "newbufferdialog.h"
#include "application.h"
#include "editor.h"
#include "scene.h"

namespace GfxPaint {

DocumentManager::DocumentManager() :
    DocumentsModel(),
    documentOpeners()
{
}

DocumentManager::~DocumentManager()
{
}

void DocumentManager::registerDocumentType(DocumentManager::DocumentOpener openFunc, const QList<QByteArray> &formats)
{
    for (const auto &format : formats) {
        documentOpeners[format].append(openFunc);
    }
}

void DocumentManager::registerEditorType(DocumentManager::EditorCreator createFunc, const QString &id)
{
    editorCreators[id].append(createFunc);
}

QString DocumentManager::openFilterString()
{
    QStringList filters;
    const auto keys = documentOpeners.keys();
    for (const auto &extension : keys) {
        filters.append(QString("%1 files (*.%2)").arg(QString(extension.toUpper()), QString(extension)));
    }
    return filters.join(";;");
}

Scene *DocumentManager::newDocument()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Scene *const document = new Scene();
    addDocument(document);
    return document;
}

Scene *DocumentManager::openDocument(const QString &filename)
{
//    Scene *document = nullptr;
//    QByteArray extension = QFileInfo(filename).completeSuffix().toUtf8();
//    if (documentOpeners.contains(extension)) {
//        auto &funcs = documentOpeners[extension];
//        for (auto func : funcs) {
//            document = func(filename);
//            if (document) break;
//        }
//    }
//    return document;
    ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Scene *const document = Scene::open(filename);
    addDocument(document);
    return document;
}

bool DocumentManager::saveDocument(Scene *const document, const QString &filename)
{
    return document->save(filename);
}

void DocumentManager::closeDocument(Scene *const document)
{
    removeDocument(document);
}

} // namespace GfxPaint
