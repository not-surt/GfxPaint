#ifndef EDITOR_H
#define EDITOR_H

#include "renderedwidget.h"

#include <QElapsedTimer>
#include <QOpenGLShaderProgram>
#include <QItemSelectionModel>
#include <cmath>
#include <tuple>
#include <set>
#include <deque>

#include "buffer.h"
#include "brush.h"
#include "rendermanager.h"
#include "scene.h"
#include "editingcontext.h"
#include "utils.h"
#include "application.h"
#include "tool.h"

namespace GfxPaint {

enum class TransformTarget {
    View,
    Object,
    Brush,
};

class Editor : public RenderedWidget
{
    Q_OBJECT

    friend class EditorInputState;

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

        bool test(const InputState &other) const {
            return (keys == other.keys) && (mouseButtons == other.mouseButtons) &&
                    ((wheelDirections == std::array<bool, 4>{{false, false, false, false}}) ||
                    ((wheelDirections[0] && other.wheelDirections[0]) ||
                     (wheelDirections[1] && other.wheelDirections[1]) ||
                     (wheelDirections[2] && other.wheelDirections[2]) ||
                     (wheelDirections[3] && other.wheelDirections[3])));
        }
    };

    explicit Editor();
    explicit Editor(Scene &scene, QWidget *parent = nullptr);
    Editor(const Editor &other);
    virtual ~Editor() override;

    virtual bool eventFilter(QObject *const watched, QEvent *const event) override;
    virtual bool event(QEvent *const event) override;

    void updateWindowTitle();
    void setDocumentFilename(const QString &filename);
    void setDocumentModified(const bool modified = true);
    QString label() const;

    virtual QSize sizeHint() const override { return QSize(64, 64); }

    EditingContext &editingContext() { return m_editingContext; }

    Buffer *getWidgetBuffer() { return RenderedWidget::widgetBuffer; }
    const Mat4 &getViewportTransform() const { return viewportTransform; }

    Tool &activeTool() {
        if (!toolStack.empty()) {
            return *toolStack.front().second.first;
        }
        else return strokeTool;
    }

    void activate();

    void insertNodes(const QList<Node *> &nodes);
    void removeSelectedNodes();
    void duplicateSelectedNodes();

    Vec2 mouseToViewport(const Vec2 &point) {
        return mouseTransform.map(point);
    }
    Vec2 viewportToWorld(const Vec2 &point) {
        return cameraTransform.inverted().map(point);
    }
    Vec2 mouseToWorld(const Vec2 &point) {
        return viewportToWorld(mouseToViewport(point));
    }
    Vec2 viewportToMouse(const Vec2 &point) {
        return mouseTransform.inverted().map(point);
    }
    Vec2 worldToViewport(const Vec2 &point) {
        return cameraTransform.map(point);
    }
    Vec2 worldToMouse(const Vec2 &point) {
        return viewportToMouse(worldToViewport(point));
    }
    float pixelSnapOffset(const PixelSnap pixelSnap, const float target, const float size) {
        switch (pixelSnap) {
        case PixelSnap::Centre: return 0.5f;
        case PixelSnap::Edge: return 0.0f;
        case PixelSnap::Both: return std::fabs(target - std::floor(target) - 0.5f) < 0.25f ? 0.5f : 1.0f;
        case PixelSnap::Auto: return std::lround(size) % 2 == 0 ? 0.0f : 0.5f;
        default: return target;
        }
    }
    Vec2 pixelSnap(const Vec2 target) {
        const Brush::Dab &dab = m_editingContext.brush().dab;
        const float offsetX = pixelSnapOffset(dab.pixelSnapX, target.x(), dab.size.x());
        const float offsetY = pixelSnapOffset(dab.pixelSnapY, target.y(), dab.size.y());
        return snap({offsetX, offsetY}, {1.0f, 1.0f}, target);
    }
    float strokeSegmentDabs(const Stroke::Point &start, const Stroke::Point &end, const Vec2 brushSize, const Vec2 absoluteSpacing, const Vec2 proportionalSpacing, const float offset, Stroke &output);
    void drawDab(const Brush::Dab &dab, const Colour &colour, BufferNode &node, const Vec2 worldPos);

    Scene &scene;
    SceneModel &model;

    StrokeTool strokeTool;
    RectTool rectTool;
    EllipseTool ellipseTool;
    ContourTool contourTool;
    PickTool pickTool;
    TransformTargetOverrideTool transformTargetOverrideTool;
    PanTool panTool;
    RotoZoomTool rotoZoomTool;
    ZoomTool zoomTool;
    RotateTool rotateTool;

    TransformTarget transformTarget() const { return m_transformMode; }
    Mat4 transform() const { return cameraTransform; }

public slots:
    void setBrush(const GfxPaint::Brush &brush);
    void setColour(const GfxPaint::Colour &colour);
    void setTransformTarget(const GfxPaint::TransformTarget m_transformMode);
    void setTransform(const Mat4 &transform);
    void updateContext();

signals:
    void brushChanged(const GfxPaint::Brush &brush);
    void colourChanged(const GfxPaint::Colour &colour);
    void paletteChanged(GfxPaint::Buffer *const palette);
    void transformModeChanged(const GfxPaint::TransformTarget m_transformMode);
    void transformChanged(const Mat4 &transform);

protected:
    void init();
    void render() override;

    EditingContext m_editingContext;

    Mat4 cameraTransform;
    TransformTarget m_transformMode;

    InputState inputState;
    Vec2 cursorPos;
    Vec2 cursorDelta;
    bool cursorOver;
    Vec2 wheelDelta;
    float pressure;
    float rotation;
    Vec2 tilt;
    QQuaternion quaternion;

    std::map<InputState, std::pair<Tool *, int>> toolSet;
    std::deque<std::pair<InputState, std::pair<Tool *, int>>> toolStack;
};

} // namespace GfxPaint

#endif // EDITOR_H
