#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include "documentsmodel.h"

namespace GfxPaint {

class Scene;
class Editor;
class MainWindow;

class DocumentManager : public DocumentsModel
{
public:
    typedef Scene *(*DocumentOpener)(const QString &filename);
    typedef Editor *(*EditorCreator)(Scene &document);

    DocumentManager();
    ~DocumentManager();

    void registerDocumentType(DocumentOpener openFunc, const QList<QByteArray> &formats);
    void registerEditorType(EditorCreator createFunc, const QString &id);
    QString openFilterString();

    Scene *newDocument();
    Scene *openDocument(const QString &filename);
    bool saveDocument(Scene *const document, const QString &filename = QString());
    void closeDocument(Scene *const document);

protected:
    QMap<QByteArray, QList<DocumentOpener>> documentOpeners;
    QMap<QString, QList<EditorCreator>> editorCreators;
};

} // namespace GfxPaint

#endif // DOCUMENTMANAGER_H
