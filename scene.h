#ifndef SCENE_H
#define SCENE_H

#include "scenemodel.h"
#include <QAbstractItemModel>

#include <QQueue>
#include <QDataStream>
#include <QFont>
#include <QMimeData>
#include <QOpenGLFramebufferObject>
#include <QStack>
#include <QTransform>
#include <QJsonObject>
//#include <png.h>

#include "buffer.h"
#include "node.h"

namespace GfxPaint {

class Traversal
{
public:    
    struct RenderTarget {
        Buffer *buffer = nullptr;
        bool indexed = false;
        const Buffer *palette = nullptr;
        QTransform transform = QTransform();
    };

    struct State {
        ~State();
        RenderTarget renderTarget;
        QTransform transform;
        QTransform parentTransform;
        const Buffer *palette = nullptr;
        //Buffer palette;
        bool rendering;
    };

    QMap<Node *, Traversal::State> *saveStates;

    QStack<RenderTarget> renderTargetStack;
    QStack<QTransform> transformStack;
    QStack<const Buffer *> paletteStack;
    bool rendering;

    Traversal() :
        saveStates(nullptr),
        renderTargetStack(), transformStack(), paletteStack()
    {}

    State state() {
//        qDebug() << "STATE!" << paletteStack.size() << paletteStack.top()->format().componentCount << paletteStack.top()->size();//////////////////////////////////////////////////////////
        //return {renderTargetStack.top(), transformStack.top(), *(++transformStack.rbegin()), paletteStack.top(), rendering};
        //return {renderTargetStack.top(), transformStack.top(), *(++transformStack.rbegin()), new Buffer(*paletteStack.top()), rendering};/////////////////////////////////////
        //return {renderTargetStack.top(), transformStack.top(), *(++transformStack.rbegin()), *paletteStack.top(), rendering};/////////////////////////////////////
        return {!renderTargetStack.isEmpty() ? renderTargetStack.top() : RenderTarget(), transformStack.top(), *(++transformStack.rbegin()), !paletteStack.isEmpty() ? new Buffer(*paletteStack.top()) : nullptr, rendering};/////////////////////////////////////
    }
};

class Scene
{
public:
    explicit Scene(const QString &filename = QString());
    explicit Scene(const Scene &other);

    Scene *clone() const;
    const QString &typeId() const;
    bool saveAs(const QString &filename);
    bool save();

    QString filename() const;
    void setFilename(const QString &filename);

    bool modified() const;
    void setModified(const bool modified = true);

    static QList<QByteArray> formats();
    static Scene *open(const QString &filename);

    QJsonObject toJsonObject() const;
    static Scene *fromJsonObject(const QJsonObject &jsonObject);
    bool save(const QString &filename = QString());

    template <typename ...Targs>
    using TraversalFunc = void (*const)(Node *const, Targs...);
    template <typename ...Targs>
    void traverse(Node *const root, TraversalFunc<Targs...> beforeChildren, TraversalFunc<Targs...> afterChildren, Targs... args) {
        QStack<QQueue<Node *>> stack;
        QQueue<Node *> queue;
        queue.enqueue(root);
        stack.push(queue);
        do {
            beforeChildren(stack.top().head(), args...);
            QQueue<Node *> queue;
            for (auto child : stack.top().head()->children) {
                if (child->enabled) {
                    queue.enqueue(child);
                }
            }
            stack.push(queue);
            while (!stack.isEmpty() && stack.top().isEmpty()) {
                stack.pop();
                if (!stack.isEmpty()) {
                    afterChildren(stack.top().dequeue(), args...);
                }
            }
        } while (!stack.isEmpty());
    }    
    static void beforeChildren(Node *const node, Traversal &traversal);
    static void afterChildren(Node *const node, Traversal &traversal);
    void renderSubGraph(Node *const node, Buffer *const buffer, const bool indexed, const Buffer *const palette, const QTransform &viewTransform, const QTransform &parentTransform, QMap<Node *, Traversal::State> *const saveStates = nullptr);
    void render(Buffer *const buffer, const bool indexed, const Buffer *const palette, const QTransform &viewTransform, QMap<Node *, Traversal::State> *const saveStates = nullptr);

    Node root;

protected:
    QFile file;
    QString m_filename;
    bool m_modified;
};

} // namespace GfxPaint

#endif // SCENE_H
