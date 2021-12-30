#ifndef EDITINGCONTEXT_H
#define EDITINGCONTEXT_H

#include <QItemSelectionModel>

#include "buffer.h"
#include "brush.h"
#include "node.h"
#include "scene.h"
#include "tool.h"

namespace GfxPaint {

class EditingContext {
public:
    enum class TransformTarget {
        View,
        Object,
        Brush,
    };
    static const std::map<TransformTarget, QString> transformTargetNames;
    enum class ToolSpace {
        Object,
        ObjectAspectCorrected,
        World,
        View,
    };
    static const std::map<ToolSpace, QString> toolSpaceNames;

    struct BufferNodeContext {
        BrushDabProgram *const dabProgram;
        ColourPickProgram *const colourPickProgram;
        BrushDabProgram *const dabStrokeBufferProgram;
        Buffer *const workBuffer;
        Buffer *const strokeBuffer;

        BufferNodeContext(BrushDabProgram *const dabProgram, ColourPickProgram *const colourPickProgram, BrushDabProgram *const dabStrokeBufferProgram, Buffer *const workBuffer, Buffer *const strokeBuffer)
            : dabProgram(dabProgram), colourPickProgram(colourPickProgram), dabStrokeBufferProgram(dabStrokeBufferProgram), workBuffer(workBuffer), strokeBuffer(strokeBuffer)
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

    void setSpace(const ToolSpace toolSpace) { m_toolSpace = toolSpace; }
    void setBlendMode(const int blendMode) {
        m_blendMode = blendMode;
        update();
    }
    void setComposeMode(const int composeMode) {
        m_composeMode = composeMode;
        update();
    }
    void setBrush(const Brush &brush) {
        m_brush = brush;
        update();
    }
    void setColour(const Colour &colour) { this->m_colour = colour; }
    void setPalette(Buffer *const palette) {
        this->m_palette = palette;
        update();
    }

    const ToolSpace &space() const { return m_toolSpace; }
    const int &blendMode() const { return m_blendMode; }
    const int &composeMode() const { return m_composeMode; }
    const Brush &brush() const { return m_brush; }
    const Colour colour() const { return m_colour; }
    Buffer *palette() const { return m_palette; }
    BufferNodeContext *bufferNodeContext(Node *const node) const {
        return m_bufferNodeContexts.value(node);
    }
    QHash<Node *, Traversal::State> &states() { return m_states; }
    QItemSelectionModel &selectionModel() { return m_selectionModel; }
    QList<Node *> &selectedNodes() { return m_selectedNodes; }

    Stroke toolStroke;
    int toolMode;
    TransformTarget transformTarget;

    struct alignas(16) ShaderStruct {
        alignas(16) Colour colour;
    };
    QString getShaderStructString() {
        return R"(
struct EditingContext {
    Colour colour;
}
)";
    }
    ShaderStruct getShaderStruct() {
        return ShaderStruct{
            m_colour,
        };
    }

private:
    Scene &scene;
    ToolSpace m_toolSpace;
    int m_blendMode;
    int m_composeMode;
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
