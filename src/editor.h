#ifndef EDITOR_H
#define EDITOR_H

#include "renderedwidget.h"

#include <QElapsedTimer>
#include <QOpenGLShaderProgram>
#include <QItemSelectionModel>
#include <QUndoCommand>
#include <QUndoStack>
#include <cmath>
#include <tuple>
#include <set>
#include <deque>
#include <valarray>

#include "buffer.h"
#include "brush.h"
#include "rendermanager.h"
#include "scene.h"
#include "editingcontext.h"
#include "utils.h"
#include "application.h"
#include "tool.h"

namespace GfxPaint {

class ToolUndoCommand : public QUndoCommand {
public:
    explicit ToolUndoCommand(const QString &text, Tool *const tool, const int mode, const std::vector<Stroke::Point> points, EditingContext *const context, const Mat4 &viewTransform)
        : QUndoCommand(text), tool(tool), mode(mode), points(points), context(*context), viewTransform(viewTransform), bufferCopy()
    {
        // TODO: calc bounds
//        const QRect &bounds = destBuffer->rect();
//        if (destBuffer && !bounds.isEmpty()) {
//            bufferCopy = Buffer(bounds.size(), destBuffer->format());
//            bufferCopy.copy(*destBuffer, bounds, {0, 0});
//        }
    }
    ~ToolUndoCommand()
    {
    }
    virtual void undo() override {
//        for (Node *node : context.selectedNodes()) {
//            const EditingContext::NodeEditingContext *const bufferNodeContext = context.nodeEditingContext(node);
//            BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
//            if (bufferNode && bufferNodeContext && bufferNodeContext->restoreBuffer) {
//                bufferNode->buffer.copy(*bufferNodeContext->restoreBuffer);
//            }
//        }
    }
    virtual void redo() override {
//        for (Node *node : context.selectedNodes()) {
//            const EditingContext::NodeEditingContext *const bufferNodeContext = context.nodeEditingContext(node);
//            BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
//            if (bufferNode && bufferNodeContext && bufferNodeContext->restoreBuffer) {
//                bufferNodeContext->restoreBuffer->copy(bufferNode->buffer);
//                //        tool->end(cursorViewportPos, point, mode);
//            }
//        }
    }

    Tool *const tool;
    const int mode;
    const std::vector<Stroke::Point> points;
    EditingContext context;
    const Mat4 viewTransform;
    Buffer bufferCopy;
};

class Editor : public RenderedWidget
{
    Q_OBJECT

public:
    struct InputState {
        QSet<Qt::Key> keys;
        QSet<Qt::MouseButton> mouseButtons;
        std::array<bool, 4> wheelDirections;

        bool operator==(const InputState &rhs) const = default;
        bool operator!=(const InputState &rhs) const = default;
        bool operator<(const InputState &rhs) const {
            return std::make_tuple(keys.values(), mouseButtons.values(), wheelDirections) < std::make_tuple(rhs.keys.values(), rhs.mouseButtons.values(), rhs.wheelDirections);
        }

        bool testExact(const InputState &other) const {
            return (keys == other.keys) && (mouseButtons == other.mouseButtons) &&
                   ((wheelDirections == std::array<bool, 4>{{false, false, false, false}}) ||
                    ((wheelDirections[0] && other.wheelDirections[0]) ||
                     (wheelDirections[1] && other.wheelDirections[1]) ||
                     (wheelDirections[2] && other.wheelDirections[2]) ||
                     (wheelDirections[3] && other.wheelDirections[3])));
        }
        bool testSubset(const InputState &other) const {
            return (other.keys.contains(keys)) && (other.mouseButtons.contains(mouseButtons)) &&
                   ((wheelDirections == std::array<bool, 4>{{false, false, false, false}}) ||
                    ((wheelDirections[0] && other.wheelDirections[0]) ||
                     (wheelDirections[1] && other.wheelDirections[1]) ||
                     (wheelDirections[2] && other.wheelDirections[2]) ||
                     (wheelDirections[3] && other.wheelDirections[3])));
        }

        void combine(const InputState &other) {
            keys.unite(other.keys);
            mouseButtons.unite(other.mouseButtons);
            wheelDirections[0] = (wheelDirections[0] || other.wheelDirections[0]);
            wheelDirections[1] = (wheelDirections[1] || other.wheelDirections[1]);
            wheelDirections[2] = (wheelDirections[2] || other.wheelDirections[2]);
            wheelDirections[3] = (wheelDirections[3] || other.wheelDirections[3]);
        }
    };

//    explicit Editor();
    explicit Editor(Scene &scene, QWidget *parent = nullptr);
    Editor(const Editor &other);
    virtual ~Editor() override;

    virtual bool eventFilter(QObject *const watched, QEvent *const event) override;
    virtual bool event(QEvent *const event) override;

    void updateWindowTitle();
    void setDocumentFilename(const QString &filename);
    void setDocumentModified(const bool modified = true);
    QString label() const;
    QUndoStack *undoStack() { return qApp->documentManager.documentUndoStack(&scene); }

    virtual QSize sizeHint() const override { return QSize(64, 64); }

    EditingContext &editingContext() { return m_editingContext; }

    Buffer *getWidgetBuffer() { return RenderedWidget::widgetBuffer; }
    const Mat4 &getViewportTransform() const { return viewportTransform; }

    void activeEditingContextUpdated();

    void insertNodes(const QList<Node *> &nodes);
    void removeSelectedNodes();
    void duplicateSelectedNodes();

    static float pixelSnapOffset(const PixelSnap pixelSnap, const float target, const float size) {
        switch (pixelSnap) {
        case PixelSnap::Centre: return 0.5f;
        case PixelSnap::Edge: return 0.0f;
        case PixelSnap::Both: return std::fabs(target - std::floor(target) - 0.5f) < 0.25f ? 0.5f : 1.0f;
        case PixelSnap::Auto: return std::lround(size) % 2 == 0 ? 0.0f : 0.5f;
        default: return target;
        }
    }
    // TODO: delete? doing it in shaders?
    static Vec2 pixelSnap(EditingContext &context, const Vec2 target) {
        const Brush::Dab &dab = context.brush.dab;
        const float offsetX = pixelSnapOffset(dab.pixelSnapX, target.x(), dab.size.x());
        const float offsetY = pixelSnapOffset(dab.pixelSnapY, target.y(), dab.size.y());
        return snap({offsetX, offsetY}, {1.0f, 1.0f}, target);
    }
    static float strokeSegmentDabs(const Stroke::Point &start, const Stroke::Point &end, const Vec2 &brushSize, const Vec2 &absoluteSpacing, const Vec2 &proportionalSpacing, const float offset, Stroke &output);
    static Mat4 toolSpace(EditingContext &context, const Mat4 &viewTransform, BufferNode &node, const EditingContext::ToolSpace space);

    Scene &scene;
    SceneModel &model;

    PixelTool pixelTool;
    BrushTool brushTool;
    RectTool rectTool;
    EllipseTool ellipseTool;
    ContourTool contourTool;
    ColourPickTool pickTool;
    TransformTargetOverrideTool transformTargetOverrideTool;
    PanTool panTool;
    RotoZoomTool rotoZoomTool;
    WheelZoomTool zoomTool;
    WheelRotateTool rotateTool;

    const std::vector<Tool *> tools{
        &pixelTool, &brushTool,
        &rectTool, &ellipseTool, &contourTool,
        &pickTool,
        &transformTargetOverrideTool,
        &panTool, &rotoZoomTool, &zoomTool, &rotateTool,
    };

    const std::vector<std::pair<std::vector<EditingContext::ToolId>, EditingContext::ToolId>> toolMenuGroups{
        {{EditingContext::ToolId::Pixel, EditingContext::ToolId::Brush}, EditingContext::ToolId::Pixel},
        {{EditingContext::ToolId::RectLined, EditingContext::ToolId::RectFilled}, EditingContext::ToolId::RectLined},
        {{EditingContext::ToolId::EllipseLined, EditingContext::ToolId::EllipseFilled}, EditingContext::ToolId::EllipseLined},
        {{EditingContext::ToolId::Contour}, EditingContext::ToolId::Contour},
        {{EditingContext::ToolId::PickColourNode, EditingContext::ToolId::PickColourScene}, EditingContext::ToolId::PickColourNode},
    };
    const EditingContext::ToolId defaultTool = EditingContext::ToolId::Pixel;

    struct ToolInfo {
        QString name;
        Tool *const tool;
        int operationMode;
    };
    const std::map<EditingContext::ToolId, ToolInfo> toolInfo{
        // Selectable tools
        {EditingContext::ToolId::Pixel, {"Pixel", &pixelTool, 0}},
        {EditingContext::ToolId::Brush, {"Brush", &brushTool, 0}},
        {EditingContext::ToolId::RectLined, {"Rectangle", &rectTool,  static_cast<int>(PrimitiveTool::Mode::Lined)}},
        {EditingContext::ToolId::RectFilled, {"Filled Rectangle", &rectTool, static_cast<int>(PrimitiveTool::Mode::Filled)}},
        {EditingContext::ToolId::EllipseLined, {"Ellipse", &ellipseTool, static_cast<int>(PrimitiveTool::Mode::Lined)}},
        {EditingContext::ToolId::EllipseFilled, {"Filled Ellipse", &ellipseTool, static_cast<int>(PrimitiveTool::Mode::Filled)}},
        {EditingContext::ToolId::Contour, {"Contour", &contourTool, 0}},
        {EditingContext::ToolId::PickColourNode, {"Pick Colour From Node", &pickTool, static_cast<int>(ColourPickTool::Mode::NodeColour)}},
        {EditingContext::ToolId::PickColourScene, {"Pick Colour From Scene", &pickTool, static_cast<int>(ColourPickTool::Mode::SceneColour)}},
        // Modeless tools
        {EditingContext::ToolId::TransformPan, {"Pan", &panTool, 0}},
        {EditingContext::ToolId::TransformZoom, {"Zoom", &panTool, static_cast<int>(RotoZoomTool::Mode::Zoom)}},
        {EditingContext::ToolId::TransformRotate, {"Rotate", &rotoZoomTool, static_cast<int>(RotoZoomTool::Mode::Rotate)}},
        {EditingContext::ToolId::TransformRotoZoom, {"RotoZoom", &rotoZoomTool, static_cast<int>(RotoZoomTool::Mode::RotoZoom)}},
        {EditingContext::ToolId::WheelZoom, {"Wheel Zoom", &zoomTool, 0}},
        {EditingContext::ToolId::WheelRotate, {"Wheel Rotate", &rotateTool, 0}},
        {EditingContext::ToolId::TransformOverrideView, {"Transform Override View", &transformTargetOverrideTool, static_cast<int>(EditingContext::TransformTarget::View)}},
        {EditingContext::ToolId::TransformOverrideObject, {"Transform Override Object", &transformTargetOverrideTool, static_cast<int>(EditingContext::TransformTarget::Object)}},
        {EditingContext::ToolId::TransformOverrideBrush, {"Transform Override Brush", &transformTargetOverrideTool, static_cast<int>(EditingContext::TransformTarget::Brush)}},
        {EditingContext::ToolId::SnappingOverrideOn, {"Snapping Override On", nullptr, 1}},
        {EditingContext::ToolId::SnappingOverrideOff, {"Snapping Override Off", nullptr, 0}},
    };
    // TODO: are these actually modelss activators and should have modal switching keys here (or should that be handled buy regular actions?)
    const std::map<InputState, EditingContext::ToolId> toolSelectors{
        {{{Qt::Key_Control}, {}, {}}, EditingContext::ToolId::PickColourNode},
        {{{Qt::Key_P}, {}, {}}, EditingContext::ToolId::Pixel},
        {{{Qt::Key_B}, {}, {}}, EditingContext::ToolId::Brush},
        {{{Qt::Key_R}, {}, {}}, EditingContext::ToolId::RectLined},
        {{{Qt::Key_Shift, Qt::Key_R}, {}, {}}, EditingContext::ToolId::RectFilled},
        {{{Qt::Key_E}, {}, {}}, EditingContext::ToolId::EllipseLined},
        {{{Qt::Key_Shift, Qt::Key_E}, {}, {}}, EditingContext::ToolId::EllipseFilled},
        {{{Qt::Key_C}, {}, {}}, EditingContext::ToolId::Contour},
    };
    enum class ToolActivationMode {
        Primary,
        Secondary,
    };
    const std::map<InputState, ToolActivationMode> selectedToolActivators{
        {{{}, {Qt::LeftButton}, {}}, ToolActivationMode::Primary}, // Selected tool primary activator
        {{{}, {Qt::RightButton}, {}}, ToolActivationMode::Secondary}, // Selected tool secondary activator
    };
    const std::map<InputState, EditingContext::ToolId> modelessToolActivators{
        {{{}, {Qt::MiddleButton}, {}}, EditingContext::ToolId::TransformPan},
        {{{Qt::Key_Control}, {Qt::MiddleButton}, {}}, EditingContext::ToolId::TransformZoom},
        {{{Qt::Key_Shift}, {Qt::MiddleButton}, {}}, EditingContext::ToolId::TransformRotate},
        {{{Qt::Key_Control, Qt::Key_Shift}, {Qt::MiddleButton}, {}}, EditingContext::ToolId::TransformRotoZoom},
        {{{Qt::Key_Space}, {}, {}}, EditingContext::ToolId::TransformPan},
        {{{Qt::Key_Control, Qt::Key_Space}, {}, {}}, EditingContext::ToolId::TransformZoom},
        {{{Qt::Key_Shift, Qt::Key_Space}, {}, {}}, EditingContext::ToolId::TransformRotate},
        {{{Qt::Key_Control, Qt::Key_Shift, Qt::Key_Space}, {}, {}}, EditingContext::ToolId::TransformRotoZoom},
        {{{}, {}, {{false, false, true, true}}}, EditingContext::ToolId::WheelZoom},
        {{{Qt::Key_Shift}, {}, {{false, false, true, true}}}, EditingContext::ToolId::WheelRotate},
        {{{Qt::Key_V}, {}, {}}, EditingContext::ToolId::TransformOverrideView},
        {{{Qt::Key_O}, {}, {}}, EditingContext::ToolId::TransformOverrideObject},
        {{{Qt::Key_B}, {}, {}}, EditingContext::ToolId::TransformOverrideBrush},
    };
    const std::map<Tool *, std::map<InputState, int>> toolModeModifiers{
        {&rectTool, {
                        {{{Qt::Key_Control}, {}, {}}, 1},
                        {{{Qt::Key_Shift}, {}, {}}, 2},
                        {{{Qt::Key_Control, Qt::Key_Shift}, {}, {}}, 3}
                    }},
        {&panTool, {
                       {{{Qt::Key_Control}, {}, {}}, 1},
                       {{{Qt::Key_Shift}, {}, {}}, 2},
                       {{{Qt::Key_Control, Qt::Key_Shift}, {}, {}}, 3}
                   }},
    };

    EditingContext::TransformTarget transformTarget() const { return m_editingContext.transformTarget; }
    Mat4 transform() const { return cameraTransform; }

    EditingContext::ToolId selectedToolId() { return m_editingContext.selectedToolId; }
    EditingContext::ToolSpace toolSpace() { return m_editingContext.toolSpace; }
    int blendMode() { return m_editingContext.blendMode; }
    int composeMode() { return m_editingContext.composeMode; }

public slots:
    void setBrush(const GfxPaint::Brush &brush);
    void setColour(const GfxPaint::Colour &colour);
    void setTransformTarget(const EditingContext::TransformTarget transformTarget);
    void setTransform(const GfxPaint::Mat4 &transform);
    void updateContext();
    void setSelectedToolId(const GfxPaint::EditingContext::ToolId toolId);
    void setToolSpace(const GfxPaint::EditingContext::ToolSpace toolSpace);
    void setBlendMode(const int blendMode);
    void setComposeMode(const int composeMode);

signals:
    void brushChanged(const GfxPaint::Brush &brush);
    void colourChanged(const GfxPaint::Colour &colour);
    void paletteChanged(GfxPaint::Buffer *const palette);
    void transformTargetChanged(const EditingContext::TransformTarget transformTarget);
    void transformChanged(const GfxPaint::Mat4 &transform);
    void selectedToolIdChanged(const GfxPaint::EditingContext::ToolId toolId);
    void toolSpaceChanged(const GfxPaint::EditingContext::ToolSpace toolSpace);
    void blendModeChanged(const int blendMode);
    void composeModeChanged(const int composeMode);

protected:
    void init();
    void render() override;

    EditingContext m_editingContext;

    Mat4 cameraTransform;

    InputState inputState;
    Vec2 cursorPos;
    Vec2 cursorDelta;
    bool cursorOver;
    Vec2 wheelDelta;
    float pressure;
    float rotation;
    Vec2 tilt;
    QQuaternion quaternion;

    std::deque<std::pair<InputState, EditingContext::ToolId>> selectedToolStack;
    std::deque<std::pair<InputState, EditingContext::ToolId>> activatedToolStack;
};

} // namespace GfxPaint

Q_DECLARE_METATYPE(GfxPaint::EditingContext::ToolSpace)

#endif // EDITOR_H
