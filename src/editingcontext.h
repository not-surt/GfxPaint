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

    std::unordered_map<Node *, Traversal::State> &states() { return m_states; }
    QItemSelectionModel &selectionModel() { return m_selectionModel; }
    QList<Node *> &selectedNodes() { return m_selectedNodes; }

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
    QList<Node *> m_selectedNodes;
    std::unordered_map<Node *, Buffer *> selectedNodeRestoreBuffers;
    std::unordered_map<Node *, std::unordered_map<std::string, Program *>> selectedNodePrograms;
};

} // namespace GfxPaint

#endif // EDITINGCONTEXT_H
