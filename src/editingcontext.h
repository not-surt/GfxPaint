#ifndef EDITINGCONTEXT_H
#define EDITINGCONTEXT_H

#include <QItemSelectionModel>

#include "buffer.h"
#include "brush.h"
#include "node.h"
#include "scene.h"

namespace GfxPaint {

class EditingContext {
public:
    enum class Space {
        Object,
        ObjectAspectCorrected,
        World,
        View,
    };
    static const std::map<Space,QString> spaceNames;

    struct BufferNodeContext {
        DabProgram *const dabProgram;
        ColourPickProgram *const colourPickProgram;
        DabProgram *const dabStrokeBufferProgram;
        Buffer *const workBuffer;
        Buffer *const strokeBuffer;

        BufferNodeContext(DabProgram *const dabProgram, ColourPickProgram *const colourPickProgram, DabProgram *const dabStrokeBufferProgram, Buffer *const workBuffer, Buffer *const strokeBuffer) :
            dabProgram(dabProgram), colourPickProgram(colourPickProgram), dabStrokeBufferProgram(dabStrokeBufferProgram), workBuffer(workBuffer), strokeBuffer(strokeBuffer)
        {}
        ~BufferNodeContext() {
            delete dabProgram;
            delete colourPickProgram;
            delete dabStrokeBufferProgram;
            delete workBuffer;
            delete strokeBuffer;
        }
    };

    explicit EditingContext(Scene &scene);
    explicit EditingContext(EditingContext &other);
    ~EditingContext();

    void update();

    void setSpace(const Space space) { m_space = space; }
    void setBrush(const Brush &brush) {
        this->m_brush = brush;
        update();
    }
    void setColour(const Colour &colour) { this->m_colour = colour; }
    void setPalette(Buffer *const palette) { this->m_palette = palette; }

    const Space &space() const { return m_space; }
    const Brush &brush() const { return m_brush; }
    const Colour colour() const { return m_colour; }
    Buffer *palette() const { return m_palette; }
    BufferNodeContext *bufferNodeContext(Node *const node) const {
        return m_bufferNodeContexts.value(node);
    }
    QHash<Node *, Traversal::State> &states() { return m_states; }
    QItemSelectionModel &selectionModel() { return m_selectionModel; }
    QList<Node *> &selectedNodes() { return m_selectedNodes; }

private:
    Scene &scene;
    Space m_space;
    Brush m_brush;
    Colour m_colour;
    Buffer *m_palette;
    QHash<Node *, BufferNodeContext *> m_bufferNodeContexts;
    QHash<Node *, Traversal::State> m_states;
    QItemSelectionModel m_selectionModel;
    QList<Node *> m_selectedNodes;
};

} // namespace GfxPaint

#endif // EDITINGCONTEXT_H
