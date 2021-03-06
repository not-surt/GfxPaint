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
    struct BufferNodeContext {
        DabProgram *const dabProgram;
        ColourPickProgram *const colourPickProgram;
        Buffer *const workBuffer;
        Buffer *const strokeBuffer;

        BufferNodeContext(DabProgram *const dabProgram, ColourPickProgram *const colourPickProgram, Buffer *const workBuffer, Buffer *const strokeBuffer) :
            dabProgram(dabProgram), colourPickProgram(colourPickProgram), workBuffer(workBuffer), strokeBuffer(strokeBuffer)
        {}
        ~BufferNodeContext() {
            delete dabProgram;
            delete colourPickProgram;
            delete workBuffer;
            delete strokeBuffer;
        }
    };

    explicit EditingContext(Scene &scene);
    explicit EditingContext(EditingContext &other);
    ~EditingContext();

    void updatePrograms();

    void setBrush(const Brush &brush);
    void setColour(const QColor &colour);
    void setPalette(Buffer *const palette);

    const Brush &brush() const { return m_brush; }
    const QColor colour() const { return m_colour; }
    Buffer *palette() const { return m_palette; }
    BufferNodeContext *bufferNodeContext(Node *const node) const {
        return m_bufferNodeContexts.value(node);
    }
    QMap<Node *, Traversal::State> &states() { return m_states; }
    QItemSelectionModel &selectionModel() { return m_selectionModel; }

private:
    Scene &scene;
    Brush m_brush;
    QColor m_colour;
    Buffer *m_palette;
    QMap<Node *, BufferNodeContext *> m_bufferNodeContexts;
    QMap<Node *, Traversal::State> m_states;
    QItemSelectionModel m_selectionModel;
};

} // namespace GfxPaint

#endif // EDITINGCONTEXT_H
