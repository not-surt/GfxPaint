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

    explicit EditingContext(Scene &scene);
    explicit EditingContext(EditingContext &other);
    ~EditingContext();

    void update(Editor &editor);

    Program *toolProgram(const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat, Tool *const tool, const QString &name);

    std::unordered_map<Node *, Traversal::State> &states() { return m_states; }
    QItemSelectionModel &selectionModel() { return m_selectionModel; }
    std::vector<Node *> &selectedNodes() { return m_selectedNodes; }

    Scene &scene;
    ToolId selectedToolId;
    int toolMode;
    ToolSpace toolSpace;
    Stroke toolStroke;
    TransformTarget transformTarget;
    int blendMode;
    int composeMode;
    Brush brush;
    Colour colour;
    Buffer *palette;
    std::unordered_map<Node *, Traversal::State> m_states;
    QItemSelectionModel m_selectionModel;
    std::vector<Node *> m_selectedNodes;
    std::unordered_map<Node *, Buffer *> selectedNodeRestoreBuffers;
    std::map<std::tuple<Buffer::Format, bool, Buffer::Format, Tool *>, std::map<QString, Program *>> formatToolPrograms;
};

} // namespace GfxPaint

#endif // EDITINGCONTEXT_H
