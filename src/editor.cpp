#include "editor.h"

#include <QMouseEvent>
#include <QtMath>
//#include <QGamepadManager>
#include <cmath>
#include "application.h"
#include "utils.h"

namespace GfxPaint {

void Editor::init()
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    QObject::connect(&m_editingContext.selectionModel(), &QItemSelectionModel::selectionChanged, this, &Editor::updateContext);

//    qDebug() << QGamepadManager::instance()->isGamepadConnected(0);
//    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent, this, [](){

//    });
//    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonPressEvent, this, [](){

//    });
//    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonReleaseEvent, this, [](){

//    });
}

Editor::Editor(Scene &scene, QWidget *parent) :
    RenderedWidget(parent),
    scene(scene), model(*qApp->documentManager.documentModel(&scene)),
    strokeTool(*this), rectTool(*this), ellipseTool(*this), contourTool(*this), pickTool(*this), transformTargetOverrideTool(*this), panTool(*this), rotoZoomTool(*this), zoomTool(*this), rotateTool(*this),
    m_editingContext(scene),
    cameraTransform(), m_transformMode(),
    inputState{}, cursorPos(), cursorDelta(), cursorOver{false}, wheelDelta{}, pressure{}, rotation{}, tilt{}, quaternion{},
    toolSet{
        {{{}, {Qt::LeftButton}, {}}, {&strokeTool, 0}},
        {{{Qt::Key_R}, {Qt::LeftButton}, {}}, {&rectTool, 0}},
        {{{Qt::Key_E}, {Qt::LeftButton}, {}}, {&ellipseTool, 0}},
        {{{Qt::Key_C}, {Qt::LeftButton}, {}}, {&contourTool, 0}},
        {{{}, {}, {{false, false, true, true}}}, {&zoomTool, 0}},
        {{{Qt::Key_Control}, {Qt::LeftButton}, {}}, {&pickTool, 0}},
        {{{}, {Qt::MiddleButton}, {}}, {&panTool, 0}},
        {{{Qt::Key_Control}, {Qt::MiddleButton}, {}}, {&rotoZoomTool, static_cast<int>(RotoZoomTool::Mode::Zoom)}},
        {{{Qt::Key_Shift}, {Qt::MiddleButton}, {}}, {&rotoZoomTool, static_cast<int>(RotoZoomTool::Mode::Rotate)}},
        {{{Qt::Key_Control, Qt::Key_Shift}, {Qt::MiddleButton}, {}}, {&rotoZoomTool, static_cast<int>(RotoZoomTool::Mode::RotoZoom)}},
        {{{Qt::Key_Space}, {}, {}}, {&panTool, 0}},
        {{{Qt::Key_Control, Qt::Key_Space}, {}, {}}, {&rotoZoomTool, static_cast<int>(RotoZoomTool::Mode::Zoom)}},
        {{{Qt::Key_Shift, Qt::Key_Space}, {}, {}}, {&rotoZoomTool, static_cast<int>(RotoZoomTool::Mode::Rotate)}},
        {{{Qt::Key_Control, Qt::Key_Shift, Qt::Key_Space}, {}, {}}, {&rotoZoomTool, static_cast<int>(RotoZoomTool::Mode::RotoZoom)}},
        {{{Qt::Key_Shift}, {}, {{false, false, true, true}}}, {&rotateTool, 0}},
        {{{Qt::Key_V}, {}, {}}, {&transformTargetOverrideTool, static_cast<int>(TransformTarget::View)}},
        {{{Qt::Key_O}, {}, {}}, {&transformTargetOverrideTool, static_cast<int>(TransformTarget::Object)}},
        {{{Qt::Key_B}, {}, {}}, {&transformTargetOverrideTool, static_cast<int>(TransformTarget::Brush)}},
    },
    toolStack{}
{
    init();
}

Editor::Editor(const Editor &other) :
    RenderedWidget(other.parentWidget()),
    scene(other.scene), model(other.model),
    strokeTool(other.strokeTool), rectTool(other.rectTool), ellipseTool(other.ellipseTool), contourTool(other.contourTool), pickTool(other.pickTool), transformTargetOverrideTool(other.transformTargetOverrideTool), panTool(other.panTool), rotoZoomTool(other.rotoZoomTool), zoomTool(other.zoomTool), rotateTool(other.rotateTool),
    m_editingContext(other.scene),
    cameraTransform(other.cameraTransform), m_transformMode(other.m_transformMode),
    inputState{}, cursorPos(), cursorDelta(), cursorOver{false}, wheelDelta{}, pressure{}, rotation{}, tilt{}, quaternion{},
    toolSet(other.toolSet), toolStack{}
{
    init();
}

Editor::~Editor()
{
    qDebug() << "Editor destructor!";
}

bool Editor::eventFilter(QObject *const watched, QEvent *const event)
{
    if (!hasFocus() && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease))  {
        const QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(event);
        QKeyEvent *eventCopy = new QKeyEvent(keyEvent->type(), keyEvent->key(), keyEvent->modifiers(), keyEvent->text(), keyEvent->isAutoRepeat(), keyEvent->count());
        QApplication::postEvent(this, eventCopy);
    }
    else if (event->type() == QEvent::Wheel)  {
        const QWheelEvent *const wheelEvent = static_cast<QWheelEvent *>(event);
        // TODO: wheel wierdness!
        qDebug() << "Window Wheel" << wheelEvent->angleDelta();//////////////////
//        QWheelEvent *eventCopy = new QWheelEvent(wheelEvent->pos(), wheelEvent->globalPos(), wheelEvent->pixelDelta(), wheelEvent->angleDelta(), wheelEvent->delta(), wheelEvent->orientation(), wheelEvent->buttons(), wheelEvent->modifiers(), wheelEvent->phase(), wheelEvent->source());
//        QApplication::postEvent(this, eventCopy);
//        return true;
    }
    else return RenderedWidget::eventFilter(watched, event);

    return false;
}

bool Editor::event(QEvent *const event)
{
    bool consume = false;

    // Handle input event
    const QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(event);
    if ((event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) ||
            (event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat()) ||
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::NonClientAreaMouseButtonRelease ||
            event->type() == QEvent::NonClientAreaMouseMove ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::Wheel ||
            event->type() == QEvent::TabletPress ||
            event->type() == QEvent::TabletRelease ||
            event->type() == QEvent::TabletMove)  {
        // Keep track of press keys/buttons & mouse pos
        const QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(event);
        const QWheelEvent *const wheelEvent = static_cast<QWheelEvent *>(event);
        const QTabletEvent *const tabletEvent = static_cast<QTabletEvent *>(event);
        inputState.wheelDirections = {{false, false, false, false}};
        cursorDelta = {0.0f, 0.0f};
        wheelDelta = {0.0f, 0.0f};
        pressure = 1.0f;
        rotation = 0.0f;
        tilt = {0.0f, 0.0f};
        quaternion = QQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
        if (event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat()) {
            cursorPos = Vec2(mapFromGlobal(QCursor::pos()));
            pressure = 1.0f;
            inputState.keys.insert(static_cast<Qt::Key>(keyEvent->key()));
        }
        else if (event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) inputState.keys.remove(static_cast<Qt::Key>(keyEvent->key()));
        else if (event->type() == QEvent::MouseButtonPress) inputState.mouseButtons.insert(mouseEvent->button());
        else if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::NonClientAreaMouseButtonRelease) inputState.mouseButtons.remove(mouseEvent->button());
        else if (event->type() == QEvent::TabletPress) inputState.mouseButtons.insert(tabletEvent->button());
        else if (event->type() == QEvent::TabletRelease) inputState.mouseButtons.remove(tabletEvent->button());
        else if (event->type() == QEvent::Wheel) {
            inputState.wheelDirections = {{wheelEvent->angleDelta().x() < 0, wheelEvent->angleDelta().x() > 0, wheelEvent->angleDelta().y() < 0, wheelEvent->angleDelta().y() > 0}};
            static const float stepSize = 8 * 15;
            wheelDelta = Vec2(wheelEvent->angleDelta()) / stepSize;
        }
        if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove || event->type() == QEvent::NonClientAreaMouseMove) {
            cursorDelta = Vec2(mouseEvent->localPos()) - cursorPos;
        }
        if (event->type() == QEvent::TabletRelease || event->type() == QEvent::TabletMove) {
            cursorDelta = Vec2(tabletEvent->posF()) - cursorPos;
        }
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::NonClientAreaMouseButtonRelease || event->type() == QEvent::MouseMove || event->type() == QEvent::NonClientAreaMouseMove) {
            cursorPos = Vec2(mouseEvent->localPos());
            pressure = 1.0f;
        }
        if (event->type() == QEvent::TabletPress || event->type() == QEvent::TabletRelease || event->type() == QEvent::TabletMove) {
            cursorPos = Vec2(tabletEvent->posF());
            pressure = static_cast<float>(tabletEvent->pressure());
            rotation = static_cast<float>(tabletEvent->rotation());
            tilt = {qDegreesToRadians(static_cast<float>(tabletEvent->xTilt())), qDegreesToRadians(static_cast<float>(tabletEvent->yTilt()))};
            rotation = qRadiansToDegrees(std::atan2(std::sin(tilt.y()), std::sin(tilt.x())) + (tau<float> / 4.0f));
            quaternion = QQuaternion::fromEulerAngles(tilt.y(), tilt.x(), rotation);
        }
        Point point{Vec2(mouseToWorld(cursorPos)), static_cast<float>(pressure), quaternion};

        // Remove non-matching tools
        auto iterator = toolStack.begin();
        while (iterator != toolStack.end()) {
            const InputState &trigger = iterator->first;
            if (!trigger.test(inputState)) {
                auto &[tool, mode] = iterator->second;
                tool->end(mouseToViewport(cursorPos), point, mode);
                iterator = toolStack.erase(iterator);
                releaseMouse();
                releaseKeyboard();
                consume = true;
            }
            else
                ++iterator;
        }

        // Add matching tools
        for (auto &[trigger, value] : toolSet) {
            if (trigger.test(inputState)) {
                auto &[tool, mode] = value;
                auto pair = std::make_pair(trigger, std::make_pair(tool, mode));
                if (std::find(toolStack.begin(), toolStack.end(), pair) == toolStack.end()) {
                    toolStack.push_front(pair);
                    tool->begin(mouseToViewport(cursorPos), point, mode);
                }
                grabMouse();
                grabKeyboard();
                consume = true;
            }
        }

        {
            auto iterator = toolStack.begin();
            while (iterator != toolStack.end()) {
                auto &[tool, mode] = iterator->second;
                // Handle mouse wheel
                if (event->type() == QEvent::Wheel) {
                    auto &[tool, mode] = toolStack.front().second;
                    tool->wheel(mouseToViewport(cursorPos), wheelDelta, mode);
                    consume = true;
                }
                // Update current tools
                else if (event->type() == QEvent::MouseMove ||
                         event->type() == QEvent::NonClientAreaMouseMove ||
                         event->type() == QEvent::TabletMove) {
                    tool->update(mouseToViewport(cursorPos), point, mode);
                    consume = true;
                }
                if (tool->isExclusive()) break;
                ++iterator;
            }
        }
    }

    if (event->type() == QEvent::Enter) {
        cursorOver = true;
        consume = true;
    }
    else if (event->type() == QEvent::Leave) {
        cursorOver = false;
        consume = true;
    }
    else if (event->type() == QEvent::FocusOut ||
        event->type() == QEvent::WindowDeactivate) {
        inputState = {};
        toolStack = {};
        releaseMouse();
        cursorOver = false;
        consume = true;
    }

    if (consume) {
        event->accept();
        return true;
    }
    else return RenderedWidget::event(event);
}

void Editor::updateWindowTitle()
{
    setWindowModified(scene.modified());
    setWindowFilePath(scene.filename());
}

void Editor::setDocumentFilename(const QString &filename)
{
    scene.setFilename(filename);
    updateWindowTitle();
}

void Editor::setDocumentModified(const bool modified)
{
    scene.setModified(modified);
    updateWindowTitle();
}

QString Editor::label() const
{
    if (scene.filename().isNull()) return "unsaved";
    else return QFileInfo(scene.filename()).fileName();
}

void Editor::activate()
{
    emit brushChanged(m_editingContext.brush());
    emit transformModeChanged(m_transformMode);
    emit transformChanged(cameraTransform);
    emit colourChanged(m_editingContext.colour());
    emit paletteChanged(m_editingContext.palette());
    updateContext();
}

void Editor::insertNodes(const QList<Node *> &nodes)
{
    const QModelIndexList rows = m_editingContext.selectionModel().selectedRows();
    if (rows.isEmpty() || rows.last().parent() == QModelIndex()) {
        model.insertNodes(nodes, scene.root.children.length(), model.rootIndex);
    }
    else {
        QModelIndexList inserted = model.insertNodes(nodes, rows.last().row() + 1, rows.last().parent());
//        if (!inserted.isEmpty()) m_editingContext.selectionModel().select(inserted.last(), QItemSelectionModel::ClearAndSelect);
    }
}

void Editor::removeSelectedNodes()
{

}

void Editor::duplicateSelectedNodes()
{

}

void Editor::setBrush(const Brush &brush)
{
    if (m_editingContext.brush() != brush) {
        m_editingContext.setBrush(brush);
        emit brushChanged(m_editingContext.brush());
    }
}

void Editor::setColour(const Colour &colour)
{
    if (m_editingContext.colour() != colour) {
        m_editingContext.setColour(colour);
        emit colourChanged(m_editingContext.colour());
    }
}

void Editor::render()
{
    for (Node *node : m_editingContext.selectedNodes()) {
        const EditingContext::BufferNodeContext *const bufferNodeContext = m_editingContext.bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode && bufferNodeContext->workBuffer) {
            bufferNodeContext->workBuffer->copy(bufferNode->buffer);
            // Draw on-canvas tool preview
            if (cursorOver) {
                ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
                bufferNode->buffer.bindFramebuffer();
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);

                const Vec2 point = mouseToWorld(cursorPos);
                const Vec2 snappedPoint = pixelSnap(point);
                activeTool().onCanvasPreview(mouseToViewport(cursorPos), {mouseToWorld(cursorPos), 1.0, quaternion});
            }
        }
    }

    // Draw scene
    widgetBuffer->bindFramebuffer();
    scene.render(widgetBuffer, false, nullptr, viewportTransform * cameraTransform, &m_editingContext.states());

    // Undraw on-canvas tool preview
    for (Node *node : m_editingContext.selectedNodes()) {
        const EditingContext::BufferNodeContext *const bufferNodeContext = m_editingContext.bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode && bufferNodeContext->workBuffer) {
            if (cursorOver) {
//                bufferNodeContext->workBuffer->clear();
                bufferNode->buffer.copy(*bufferNodeContext->workBuffer);
            }
        }
    }

    LineProgram *lineProgram = new LineProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    std::vector<LineProgram::Point> points{
        {{16.0, 256.0f}, 0.0f, 0.0f, {{0.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
        {{16.0, 16.0f}, 0.0f, 0.0f, {{1.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
        {{256.0, 16.0f}, 16.0f, 0.0f, {{0.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
        {{256.0, 256.0f}, 32.0f, 0.0f, {{0.0f, 1.0f, 1.0f, 1.0f}, INDEX_INVALID}},
        {{16.0, 256.0f}, 16.0f, 0.0f, {{1.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
        {{128.0, 128.0f}, 8.0f, 0.0f, {{0.0f, 0.0f, 0.0f, 0.5f}, INDEX_INVALID}},
        {{16.0, 16.0f}, 0.0f, 0.0f, {{0.0f, 0.0f, 0.0f, 0.0f}, INDEX_INVALID}},
        {{16.0, 16.0f}, 0.0f, 0.0f, {{0.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
    };
    lineProgram->render(points, {{1.0f, 0.0f, 0.0f, 1.0f}, INDEX_INVALID}, viewportTransform * cameraTransform, widgetBuffer, nullptr);

    activeTool().onTopPreview(mouseToViewport(cursorPos), {mouseToWorld(cursorPos), 1.0, quaternion});
}

float Editor::strokeSegmentDabs(const Stroke::Point &start, const Stroke::Point &end, const Vec2 dabSize, const Vec2 absoluteSpacing, const Vec2 proportionalSpacing, const float offset, Stroke &output) {
    auto ellipsePolar = [](const float a, const float b, const float theta){
        return (a * b) / std::sqrt(std::pow(a, 2.0f) * std::pow(std::sin(theta), 2.0f) + std::pow(b, 2.0f) * std::pow(std::cos(theta), 2.0f));
    };

    const Vec2 spacing{absoluteSpacing + proportionalSpacing * dabSize};

    const float a = spacing.x() / 2.0f;
    const float b = spacing.y() / 2.0f;
    const float theta = std::atan2(end.pos.y() - start.pos.y(), end.pos.x() - start.pos.x());
    float increment = ellipsePolar(a, b, theta + ((2.0f * pi<float>) / 360.0f) * -start.quaternion.scalar()) * 2.0f;

    const float incrementScale = 1.0;
    float pos = offset * increment;
    if (pos == 0.0f) {
        output.add(start);
        pos += increment;
    }
    const Vec2 delta = end.pos - start.pos;
    const float length = delta.length();
    while (pos < length) {
        output.add(Stroke::interpolate(start, end, pos / length));
        pos += std::max(increment, 1.0f);
        increment *= incrementScale;
    }

    return (pos - length) / increment;
}

void Editor::drawDab(const Brush::Dab &dab, const Colour &colour, BufferNode &node, const Vec2 worldPos)
{
    Q_ASSERT( m_editingContext.bufferNodeContext(&node));

    const Traversal::State &state = m_editingContext.states().value(&node);

    Mat4 worldTransform;
    worldTransform.translate(worldPos.x(), worldPos.y());

    const Mat4 &transform = Mat4(state.transform);
    Mat4 spaceTransform;
    switch (dab.space) {
    case Space::Object: {
        spaceTransform = Mat4();
    } break;
    case Space::ObjectAspectCorrected: {
        spaceTransform = node.pixelAspectRatioTransform().inverted();
    } break;
    case Space::World: {
        spaceTransform = transform.inverted();
    } break;
    case Space::View: {
        spaceTransform = (cameraTransform * transform).inverted();
    } break;
    }
    const Vec2 spaceOffset = spaceTransform.map(Vec2(0.0f, 0.0f));
    Mat4 spaceOffsetTransform;
    spaceOffsetTransform.translate(-spaceOffset);
    spaceTransform = spaceOffsetTransform * spaceTransform;
    const Vec2 objectSpacePos = transform.inverted().map(worldPos);
    Mat4 objectSpaceTransform;
    objectSpaceTransform.translate(objectSpacePos);

    const Buffer *const palette = m_editingContext.states()[&node].palette;
    EditingContext::BufferNodeContext *const bufferNodeContext = m_editingContext.bufferNodeContext(&node);
    bufferNodeContext->dabProgram->render(dab, colour, GfxPaint::viewportTransform(node.buffer.size()) * objectSpaceTransform * spaceTransform, &node.buffer, palette);
}

void Editor::setTransformTarget(const TransformTarget transformMode)
{
    if (this->m_transformMode != transformMode) {
        this->m_transformMode = transformMode;
        emit transformModeChanged(this->m_transformMode);
    }
}

void Editor::setTransform(const Mat4 &transform)
{
    if (this->cameraTransform != transform) {
        this->cameraTransform = transform;
        emit transformChanged(this->cameraTransform);
        update();
    }
}

void Editor::updateContext()
{
    m_editingContext.update();

    Buffer *palette = nullptr;
    for (Node *node : m_editingContext.selectedNodes()) {
        const Traversal::State &state = m_editingContext.states().value(node);
        if (state.palette) palette = state.palette;
    }
    emit paletteChanged(palette);
}

} // namespace GfxPaint
