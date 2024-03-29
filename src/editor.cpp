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
    pixelTool(), brushTool(), rectTool(), ellipseTool(), contourTool(), pickTool(), transformTargetOverrideTool(*this), panTool(*this), rotoZoomTool(*this), zoomTool(*this), rotateTool(*this),
    m_editingContext(scene),
    cameraTransform(),
    inputState{}, cursorPos(), cursorDelta(), cursorOver{false}, wheelDelta{}, pressure{}, rotation{}, tilt{}, quaternion{},
    selectedToolStack{}, activatedToolStack{}
{
    init();
}

Editor::Editor(const Editor &other) :
    RenderedWidget(other.parentWidget()),
    scene(other.scene), model(other.model),
    pixelTool(other.pixelTool), brushTool(other.brushTool), rectTool(other.rectTool), ellipseTool(other.ellipseTool), contourTool(other.contourTool), pickTool(other.pickTool), transformTargetOverrideTool(other.transformTargetOverrideTool), panTool(other.panTool), rotoZoomTool(other.rotoZoomTool), zoomTool(other.zoomTool), rotateTool(other.rotateTool),
    m_editingContext(other.scene),
    cameraTransform(other.cameraTransform),
    inputState{}, cursorPos(), cursorDelta(), cursorOver{false}, wheelDelta{}, pressure{}, rotation{}, tilt{}, quaternion{},
    toolSelectors(other.toolSelectors), selectedToolActivators(other.selectedToolActivators), modelessToolActivators(other.modelessToolActivators), toolModeModifiers(other.toolModeModifiers),
    selectedToolStack(other.selectedToolStack), activatedToolStack(other.activatedToolStack)
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
//    else if (event->type() == QEvent::Wheel)  {
//        const QWheelEvent *const wheelEvent = static_cast<QWheelEvent *>(event);
//        // TODO: wheel wierdness!
//        qDebug() << "Window Wheel" << wheelEvent->angleDelta();//////////////////
//        QWheelEvent *eventCopy = new QWheelEvent(wheelEvent->position(), wheelEvent->globalPosition(), wheelEvent->pixelDelta(), wheelEvent->angleDelta(), wheelEvent->buttons(), wheelEvent->modifiers(), wheelEvent->phase(), wheelEvent->inverted(), wheelEvent->source());
//        QApplication::postEvent(this, eventCopy);
//    }
    else return RenderedWidget::eventFilter(watched, event);

    return false;
}

bool Editor::event(QEvent *const event)
{
    bool consume = false;
    bool inputStateChanged = false;

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
            inputStateChanged = true;
        }
        else if (event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) {
            inputState.keys.remove(static_cast<Qt::Key>(keyEvent->key()));
            inputStateChanged = true;
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            inputState.mouseButtons.insert(mouseEvent->button());
            inputStateChanged = true;
        }
        else if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::NonClientAreaMouseButtonRelease) {
            inputState.mouseButtons.remove(mouseEvent->button());
            inputStateChanged = true;
        }
        else if (event->type() == QEvent::TabletPress) {
            inputState.mouseButtons.insert(tabletEvent->button());
            inputStateChanged = true;
        }
        else if (event->type() == QEvent::TabletRelease) {
            inputState.mouseButtons.remove(tabletEvent->button());
            inputStateChanged = true;
        }
        else if (event->type() == QEvent::Wheel) {
            inputState.wheelDirections = {{wheelEvent->angleDelta().x() < 0, wheelEvent->angleDelta().x() > 0, wheelEvent->angleDelta().y() < 0, wheelEvent->angleDelta().y() > 0}};
            inputStateChanged = true;
            static const float stepSize = 8.0f * 15.0f;
            wheelDelta = Vec2(wheelEvent->angleDelta()) / stepSize;
        }

        if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove || event->type() == QEvent::NonClientAreaMouseMove) {
            cursorDelta = Vec2(mouseEvent->position()) - cursorPos;
            inputStateChanged = true;
        }
        if (event->type() == QEvent::TabletRelease || event->type() == QEvent::TabletMove) {
            cursorDelta = Vec2(tabletEvent->position()) - cursorPos;
            inputStateChanged = true;
        }
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::NonClientAreaMouseButtonRelease || event->type() == QEvent::MouseMove || event->type() == QEvent::NonClientAreaMouseMove) {
            cursorPos = Vec2(mouseEvent->position());
            pressure = 1.0f;
            inputStateChanged = true;
        }
        if (event->type() == QEvent::TabletPress || event->type() == QEvent::TabletRelease || event->type() == QEvent::TabletMove) {
            cursorPos = Vec2(tabletEvent->position());
            pressure = static_cast<float>(tabletEvent->pressure());
            rotation = static_cast<float>(tabletEvent->rotation());
            tilt = {qDegreesToRadians(static_cast<float>(tabletEvent->xTilt())), qDegreesToRadians(static_cast<float>(tabletEvent->yTilt()))};
            rotation = qRadiansToDegrees(std::atan2(std::sin(tilt.y()), std::sin(tilt.x())) + (tau<float> / 4.0f));
            quaternion = QQuaternion::fromEulerAngles(tilt.y(), tilt.x(), rotation);
            inputStateChanged = true;
        }

        const Vec2 cursorViewportPos = mouseTransform * cursorPos;
        const Vec2 cursorWorldPos = cameraTransform.inverted() * cursorViewportPos;

        if (inputStateChanged) {
            selectedToolStack.clear();
            if (m_editingContext.selectedToolId != EditingContext::ToolId::Invalid) {
                auto pair = std::make_pair(InputState(), m_editingContext.selectedToolId);
                selectedToolStack.push_front(pair);
            }
            for (const auto &[trigger, id] : toolSelectors) {
                if (trigger.testSubset(inputState)) {
                    auto pair = std::make_pair(trigger, id);
                    selectedToolStack.push_front(pair);
                }
            }

            // Build toolset
            auto toolSet = modelessToolActivators;
            for (const auto &[activatorTrigger, mode] : selectedToolActivators) {
                if (activatorTrigger.testSubset(inputState)) {
                    for (const auto &[selectorTrigger, id] : selectedToolStack) {
                        InputState combinedTrigger = activatorTrigger;
                        combinedTrigger.combine(selectorTrigger);
                        if (combinedTrigger.testExact(inputState)) {
                            toolSet.insert(std::make_pair(combinedTrigger, id));
                        }
                    }
                }
            }

            // Remove non-matching tools
            auto iterator = activatedToolStack.begin();
            while (iterator != activatedToolStack.end()) {
                const auto &[trigger, id] = *iterator;
                if (!trigger.testExact(inputState)) {
                    const ToolInfo &info = toolInfo.at(id);
                    m_editingContext.toolStroke.add(cursorWorldPos, pressure, quaternion);
                    m_editingContext.toolMode = info.operationMode;
                    info.tool->end(m_editingContext, transform());
                    if (info.tool->updatesContext()) {
                        activeEditingContextUpdated();
                    }
                    iterator = activatedToolStack.erase(iterator);
                    releaseMouse();
                    releaseKeyboard();
                    consume = true;
                    undoStack()->push(new ToolUndoCommand(info.name, info.tool, info.operationMode, {}, &m_editingContext, transform()));////////////////////////////////////
                }
                else
                    ++iterator;
            }

            // Activate matching tools
            for (const auto &[trigger, id] : toolSet) {
                if (trigger.testExact(inputState)) {
                    auto pair = std::make_pair(trigger, id);
                    if (std::find(activatedToolStack.begin(), activatedToolStack.end(), pair) == activatedToolStack.end()) {
                        activatedToolStack.push_front(pair);
                        const ToolInfo &info = toolInfo.at(id);
                        m_editingContext.toolStroke = {};
                        m_editingContext.toolStroke.add(cursorWorldPos, pressure, quaternion);
                        m_editingContext.toolMode = info.operationMode;
                        info.tool->begin(m_editingContext, transform());
                        if (info.tool->updatesContext()) {
                            activeEditingContextUpdated();
                        }
//                        grabMouse();
//                        grabKeyboard();
                        consume = true;
                    }
                }
            }
        }

        {
            for (auto &[inputState, toolId] : activatedToolStack) {
                const ToolInfo &info = toolInfo.at(toolId);
                // Handle mouse wheel
                if (event->type() == QEvent::Wheel) {
                    m_editingContext.toolMode = info.operationMode;
                    info.tool->wheel(m_editingContext, transform(), wheelDelta);
                    if (info.tool->updatesContext()) {
                        activeEditingContextUpdated();
                    }
                    consume = true;
                }
                // Update current tools
                if (event->type() == QEvent::MouseMove ||
                         event->type() == QEvent::NonClientAreaMouseMove ||
                         event->type() == QEvent::TabletMove) {
                    m_editingContext.toolStroke.add(cursorWorldPos, pressure, quaternion);
                    m_editingContext.toolMode = info.operationMode;
                    info.tool->update(m_editingContext, transform());
                    if (info.tool->updatesContext()) {
                        activeEditingContextUpdated();
                    }
                    consume = true;
                }
                if (info.tool->isExclusive()) break;
            }
        }

        // Add points for preview
        if (activatedToolStack.empty()/* && !selectedToolStack.empty()*/) {
            m_editingContext.toolStroke = {};
            m_editingContext.toolStroke.add(cursorWorldPos, pressure, quaternion);
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
        activatedToolStack = {};
        selectedToolStack = {};
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

void Editor::activeEditingContextUpdated()
{
    emit brushChanged(m_editingContext.brush);
    emit transformTargetChanged(m_editingContext.transformTarget);
    emit transformChanged(cameraTransform);
    emit colourChanged(m_editingContext.colour);
    emit paletteChanged(m_editingContext.palette);
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

void Editor::setSelectedToolId(const EditingContext::ToolId toolId)
{
    if (m_editingContext.selectedToolId != toolId) {
        m_editingContext.selectedToolId = toolId;
        m_editingContext.update(*this);
        emit selectedToolIdChanged(toolId);
    }
}

void Editor::setToolSpace(const EditingContext::ToolSpace toolSpace)
{
    if (m_editingContext.toolSpace != toolSpace) {
        m_editingContext.toolSpace = toolSpace;
        emit toolSpaceChanged(toolSpace);
    }
}

void Editor::setBlendMode(const int blendMode)
{
    if (m_editingContext.blendMode != blendMode) {
        m_editingContext.blendMode = blendMode;
        m_editingContext.update(*this);
        emit blendModeChanged(blendMode);
    }
}

void Editor::setComposeMode(const int composeMode)
{
    if (m_editingContext.composeMode != composeMode) {
        m_editingContext.composeMode = composeMode;
        m_editingContext.update(*this);
        emit composeModeChanged(composeMode);
    }
}

void Editor::setBrush(const Brush &brush)
{
    if (m_editingContext.brush != brush) {
        m_editingContext.brush = brush;
        m_editingContext.update(*this);
        emit brushChanged(brush);
    }
}

void Editor::setColour(const Colour &colour)
{
    if (m_editingContext.colour != colour) {
        m_editingContext.colour = colour;
        emit colourChanged(colour);
    }
}

void Editor::render()
{
    Tool *onCanvasPreviewTool = nullptr;
    bool onCanvasPreviewIsActive = false;
    int onCanvasPreviewMode = 0;
    if (activatedToolStack.size() > 0) {
        const ToolInfo &info = toolInfo.at(activatedToolStack.front().second);
        onCanvasPreviewTool = info.tool;
        onCanvasPreviewIsActive = true;
        onCanvasPreviewMode = info.operationMode;
    }
    else if (selectedToolStack.size() > 0) {
        const ToolInfo &info = toolInfo.at(selectedToolStack.front().second);
        onCanvasPreviewTool = info.tool;
        onCanvasPreviewMode = info.operationMode;
    }

    for (Node *node : m_editingContext.selectedNodes()) {
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode && m_editingContext.selectedNodeRestoreBuffers[node]) {
            // Draw on-canvas tool preview
            if (cursorOver && onCanvasPreviewTool) {
                m_editingContext.selectedNodeRestoreBuffers[node]->copy(bufferNode->buffer);
                ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
                bufferNode->buffer.bindFramebuffer();
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);

                m_editingContext.toolMode = onCanvasPreviewMode;
                onCanvasPreviewTool->onCanvasPreview(m_editingContext, transform(), onCanvasPreviewIsActive);
            }
        }
    }

    // Draw scene
    widgetBuffer->bindFramebuffer();
    scene.render(widgetBuffer, false, nullptr, viewportTransform * cameraTransform, &m_editingContext.states());

    for (Node *node : m_editingContext.selectedNodes()) {
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode && m_editingContext.selectedNodeRestoreBuffers[node]) {
            // Undraw on-canvas tool preview
            if (cursorOver && onCanvasPreviewTool) {
                bufferNode->buffer.copy(*m_editingContext.selectedNodeRestoreBuffers[node]);
            }
        }
    }

//    LineProgram *lineProgram = new LineProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
//    std::vector<LineProgram::Point> points{
//        {{16.0, 256.0f}, 0.0f, 0.0f, {{0.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
//        {{16.0, 16.0f}, 0.0f, 0.0f, {{1.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
//        {{256.0, 16.0f}, 16.0f, 0.0f, {{0.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
//        {{256.0, 256.0f}, 32.0f, 0.0f, {{0.0f, 1.0f, 1.0f, 1.0f}, INDEX_INVALID}},
//        {{16.0, 256.0f}, 16.0f, 0.0f, {{1.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
//        {{128.0, 128.0f}, 8.0f, 0.0f, {{0.0f, 0.0f, 0.0f, 0.5f}, INDEX_INVALID}},
//        {{16.0, 16.0f}, 0.0f, 0.0f, {{0.0f, 0.0f, 0.0f, 0.0f}, INDEX_INVALID}},
//        {{16.0, 16.0f}, 0.0f, 0.0f, {{0.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}},
//    };
//    lineProgram->render(points, {{1.0f, 0.0f, 0.0f, 1.0f}, INDEX_INVALID}, viewportTransform * cameraTransform, widgetBuffer, nullptr);

//    std::vector<Stroke::Point> strokePoints{
//        {{16.0, 16.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 1.0f},
//        {{160.0, 16.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 0.0f},
//        {{160.0, 160.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 0.0f},
//        {{16.0, 160.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 0.0f},
//        {{16.0, 16.0f}, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 1.0f},
//    };

//    PixelLineProgram *pixelLineProgram = new PixelLineProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
//    pixelLineProgram->render(strokePoints, {{1.0f, 0.0f, 1.0f, 1.0f}, INDEX_INVALID}, viewportTransform * cameraTransform, Mat4(), widgetBuffer, nullptr);

//    BrushDabProgram *brushDabProgram = new BrushDabProgram(m_editingContext.brush().dab.type, m_editingContext.brush().dab.metric, RenderedWidget::format, false, Buffer::Format(), m_editingContext.blendMode(), m_editingContext.composeMode());
//    brushDabProgram->render(strokePoints, m_editingContext.brush().dab, m_editingContext.colour(), viewportTransform * cameraTransform, Mat4(), widgetBuffer, nullptr);

//    SmoothQuadProgram *smoothQuadProgram = new SmoothQuadProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
//    std::vector<vec2> smoothQuadPoints{
//        {16.0, 16.0f},
//        {160.0, 16.0f},
//        {160.0, 160.0f},
//        {66.0, 100.0f},
//   };
//    smoothQuadProgram->render(smoothQuadPoints, viewportTransform * cameraTransform, widgetBuffer, nullptr);

    Tool *onTopPreviewTool = nullptr;
    bool onTopPreviewIsActive = false;
    int onTopPreviewMode = 0;
    if (activatedToolStack.size() > 0) {
        const ToolInfo &info = toolInfo.at(activatedToolStack.front().second);
        onTopPreviewTool = info.tool;
        onTopPreviewIsActive = true;
        onTopPreviewMode = info.operationMode;
    }
    else if (selectedToolStack.size() > 0) {
        const ToolInfo &info = toolInfo.at(selectedToolStack.front().second);
        onTopPreviewTool = info.tool;
        onTopPreviewMode = info.operationMode;
    }
    m_editingContext.toolMode = onTopPreviewMode;
    if (onTopPreviewTool) onTopPreviewTool->onTopPreview(*this, m_editingContext, transform(), onTopPreviewIsActive);
}

float Editor::strokeSegmentDabs(const Stroke::Point &start, const Stroke::Point &end, const Vec2 &dabSize, const Vec2 &absoluteSpacing, const Vec2 &proportionalSpacing, const float offset, Stroke &output) {
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

Mat4 Editor::toolSpace(EditingContext &context, const Mat4 &viewTransform, BufferNode &node, const EditingContext::ToolSpace space)
{
    const Traversal::State &state = context.states().at(&node);

    Mat4 toolSpaceTransform;
    switch (space) {
    case EditingContext::ToolSpace::Object: {
        toolSpaceTransform = state.transform.inverted(); // World-space to object-space
    } break;
    case EditingContext::ToolSpace::ObjectAspectCorrected: {
        // TODO: wrong!
        toolSpaceTransform = state.transform.inverted() * node.pixelAspectRatioTransform(); // Object-space to aspect-corrected object-space
    } break;
    case EditingContext::ToolSpace::World: {
        toolSpaceTransform = Mat4(); // World-space to world-space
    } break;
    case EditingContext::ToolSpace::View: {
        toolSpaceTransform = viewTransform; // World-space to view-space
    } break;
    }
    return toolSpaceTransform;
}

void Editor::setTransformTarget(const EditingContext::TransformTarget transformTarget)
{
    if (m_editingContext.transformTarget != transformTarget) {
        m_editingContext.transformTarget = transformTarget;
        emit transformTargetChanged(transformTarget);
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
    m_editingContext.update(*this);

    Buffer *palette = nullptr;
    for (Node *node : m_editingContext.selectedNodes()) {
        const Traversal::State &state = m_editingContext.states().at(node);
        if (state.palette) palette = state.palette;
    }
    emit paletteChanged(palette);
}

} // namespace GfxPaint
