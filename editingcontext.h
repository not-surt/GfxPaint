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
    explicit EditingContext(Scene &scene);
    explicit EditingContext(EditingContext &other);
    ~EditingContext();

    void updatePrograms();
    void updateWorkBuffers();

    void setBrush(const Brush &brush);
    void setColour(const QColor &colour);

    const Brush &brush() const { return m_brush; }
    const QColor colour() const { return m_colour; }
    const QMap<Node *, DabProgram *> &dabPrograms() const { return m_dabPrograms; }
    const QMap<Node *, ColourPickProgram *> &colourPickPrograms() const { return m_colourPickPrograms; }
    const QMap<Node *, Buffer *> &workBuffers() const { return m_workBuffers; }
    QMap<Node *, Traversal::State> &states() { return m_states; }
    QItemSelectionModel &selectionModel() { return m_selectionModel; }

private:
    Scene &scene;
    Brush m_brush;
    QColor m_colour;
    QMap<Node *, DabProgram *> m_dabPrograms;
    QMap<Node *, ColourPickProgram *> m_colourPickPrograms;
    QMap<Node *, Buffer *> m_workBuffers;
    QMap<Node *, Traversal::State> m_states;
    QItemSelectionModel m_selectionModel;
};

} // namespace GfxPaint

#endif // EDITINGCONTEXT_H
