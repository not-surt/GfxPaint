﻿#ifndef EDITINGCONTEXT_H
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
    enum class ToolId {
        Invalid = -1,
        Pixel,
        Brush,
        RectLined,
        RectFilled,
        EllipseLined,
        EllipseFilled,
        Contour,
        FillPour,
        FillFlood,
        FillReplace,
        PickColourNode,
        PickColourScene,
        PickNode,
        TransformPan,
        TransformZoom,
        TransformRotate,
        TransformRotoZoom,
        // Modeless tools
        WheelZoom,
        WheelRotate,
        TransformOverrideView,
        TransformOverrideObject,
        TransformOverrideBrush,
        SnappingOverrideOn,
        SnappingOverrideOff,
    };
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

    struct NodeEditingContext {
        PixelLineProgram *const pixelLineProgram;
        BrushDabProgram *const brushDabProgram;
        ColourPickProgram *const colourPickProgram;
        Buffer *const restoreBuffer;
        std::map<std::string, Program *> programs;

        NodeEditingContext(EditingContext *const context, Node *const node, Tool *const tool, PixelLineProgram *const pixelLineProgram, BrushDabProgram *const brushDabProgram, ColourPickProgram *const colourPickProgram, Buffer *const restoreBuffer);
        ~NodeEditingContext();
    };

    explicit EditingContext(Scene &scene);
    explicit EditingContext(EditingContext &other);
    ~EditingContext();

    void update();

    void setSelectedToolId(const ToolId selectedToolId) {
        m_selectedToolId = selectedToolId;
        update();
    }
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

    const ToolId &selectedToolId() const { return m_selectedToolId; }
    const ToolSpace &space() const { return m_toolSpace; }
    const int &blendMode() const { return m_blendMode; }
    const int &composeMode() const { return m_composeMode; }
    const Brush &brush() const { return m_brush; }
    const Colour colour() const { return m_colour; }
    Buffer *palette() const { return m_palette; }
    NodeEditingContext *nodeEditingContext(Node *const node) const {
        return m_nodeEditingContexts.value(node);
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
    ToolId m_selectedToolId;
    ToolSpace m_toolSpace;
    int m_blendMode;
    int m_composeMode;
    Brush m_brush;
    Colour m_colour;    
    Buffer *m_palette;
    QHash<Node *, NodeEditingContext *> m_nodeEditingContexts;
    QHash<Node *, Traversal::State> m_states;
    QItemSelectionModel m_selectionModel;
    QList<Node *> m_selectedNodes;
};

} // namespace GfxPaint

#endif // EDITINGCONTEXT_H
