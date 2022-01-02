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
#include <QJsonObject>
#include <unordered_set>

#include "buffer.h"
#include "node.h"

namespace GfxPaint {

class Editor;

class Traversal
{
public:    
    struct RenderTarget {
        Buffer *buffer = nullptr;
        bool indexed = false;
        const Buffer *palette = nullptr;
        Mat4 transform = Mat4();
    };

    struct State {
        ~State();
        RenderTarget renderTarget;
        Mat4 transform;
        Mat4 parentTransform;
        Buffer *palette = nullptr;
        //Buffer palette;
        bool rendering;
    };

    std::unordered_map<Node *, Traversal::State> *saveStates;

    QStack<RenderTarget> renderTargetStack;
    QStack<Mat4> transformStack;
    QStack<const Buffer *> paletteStack;
    bool rendering;

    Traversal() :
        saveStates(nullptr),
        renderTargetStack(), transformStack(), paletteStack()
    {}

    State state() {
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

// MSVC don't like
    template <typename ...Targs>
    using TraversalFunc = void (*const)(Node *const, Targs...);
    template <typename ...Targs>
    static void traverse(Node *const root, TraversalFunc<Targs...> beforeChildren, TraversalFunc<Targs...> afterChildren, Targs... args) {
//    static void traverse(Node *const root, Traversal &traversal) {
        QStack<QQueue<Node *>> stack;
        QQueue<Node *> queue;
        queue.enqueue(root);
        stack.push(queue);
        do {
            beforeChildren(stack.top().head(), args...);
//            beforeChildren(stack.top().head(), traversal);
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
//                    afterChildren(stack.top().dequeue(), traversal);
                }
            }
        } while (!stack.isEmpty());
    }
    static void beforeChildren(Node *const node, Traversal &traversal);
    static void afterChildren(Node *const node, Traversal &traversal);
    void renderSubGraph(Node *const node, Buffer *const buffer, const bool indexed, const Buffer *const palette, const Mat4 &viewTransform, const Mat4 &parentTransform, std::unordered_map<Node *, Traversal::State> * const saveStates = nullptr);
    void render(Buffer *const buffer, const bool indexed, const Buffer *const palette, const Mat4 &viewTransform, std::unordered_map<Node *, Traversal::State> * const saveStates = nullptr);

    Node root;

    std::unordered_map<Buffer *, std::pair<GLuint, std::unordered_set<const Editor *>>> bufferEditors;
    void bufferAddEditor(Buffer *const buffer, const Editor *const editor);
    void bufferRemoveEditor(Buffer *const buffer, const Editor *const editor);

protected:
    QFile file;
    QString m_filename;
    bool m_modified;
};

} // namespace GfxPaint

#endif // SCENE_H
