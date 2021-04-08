#ifndef DOCUMENTSMODEL_H
#define DOCUMENTSMODEL_H

#include <QAbstractListModel>

namespace GfxPaint {

class Scene;
class SceneModel;
class Editor;

class DocumentsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    DocumentsModel();
    virtual ~DocumentsModel();

    const QList<Scene *> &documents() const;
    int documentCount() const;
    QModelIndex documentModelIndex(Scene *const document);
    void addDocument(Scene *const document);
    void removeDocument(Scene *const document);

    SceneModel *documentModel(Scene *const document) const;
    QList<Editor *> documentEditors(Scene *const document) const;
    int documentEditorCount(Scene *const document) const;
    void addEditor(Editor *const editor);
    void removeEditor(Editor *const editor);

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

signals:
    void requestCloseDocument(GfxPaint::Scene *const document);

protected:
    QList<Scene *> m_documents;
    QMap<Scene *, SceneModel *> m_documentModels;
    QMap<Scene *, QList<Editor *>> m_documentEditors;
};

} // namespace GfxPaint

#endif // DOCUMENTSMODEL_H
