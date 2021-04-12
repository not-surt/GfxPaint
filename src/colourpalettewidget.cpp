#include "colourpalettewidget.h"

#include "application.h"

namespace GfxPaint {

ColourPaletteWidget::ColourPaletteWidget(QWidget *const parent) :
    RenderedWidget(parent),
    m_swatchSize(16, 16), m_columnCount(16),
    program(nullptr), pickProgram(nullptr), selectionProgram(nullptr),
    m_palette(nullptr), m_selection(nullptr), m_ordering(nullptr)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

ColourPaletteWidget::~ColourPaletteWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
    delete pickProgram;
    delete selectionProgram;
    delete m_selection;
    delete m_ordering;
}

QSize ColourPaletteWidget::sizeHint() const { return QSize(512, 1024); }

void ColourPaletteWidget::setPalette(const Buffer *const palette)
{
    m_palette = palette;
    QList<Program *> oldPrograms = {program, selectionProgram, pickProgram};
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    if (m_palette) {
        delete m_selection;
        m_selection = new Buffer(m_palette->size(), BufferData::Format(BufferData::Format::ComponentType::UInt, 1, 1));
        m_selection->clearUInt();
        delete m_ordering;
        m_ordering = new Buffer(m_palette->size(), BufferData::Format(BufferData::Format::ComponentType::UInt, 4, 1));
        m_ordering->clearUInt();
        program = new ColourPaletteProgram(RenderedWidget::format, 0, m_palette->format());
        selectionProgram = new ColourPaletteProgram(RenderedWidget::format, 0, m_selection->format());
        pickProgram = new ColourPalettePickProgram(m_palette->format());
    }
    else {
        program = nullptr;
        selectionProgram = nullptr;
        pickProgram = nullptr;
    }
    qDeleteAll(oldPrograms);
    update();
}

void ColourPaletteWidget::setColumnCount(const int columns)
{
    m_columnCount = columns;
    update();
}

void ColourPaletteWidget::setSwatchSize(const QSize &size)
{
    m_swatchSize = size;
    update();
}

void ColourPaletteWidget::mouseEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_palette) {
            const QVector2D pos = QVector2D((float)clamp(0, width(), event->pos().x()), (float)clamp(0, height(), event->pos().y()));
            Colour colour = pickProgram->pick(m_palette, size(), m_swatchSize, cells(), pos);
            // TODO: painting with valid index screwy
            colour.index = INDEX_INVALID;///////////////////////////////////////////////////
            emit colourPicked(colour);
            event->accept();
            return;
        }
    }

    event->ignore();
}

void ColourPaletteWidget::initializeGL()
{
    RenderedWidget::initializeGL();

    setPalette(m_palette);
}

void ColourPaletteWidget::render()
{
    if (m_palette) {
        program->render(m_palette, size(), m_swatchSize, cells(), RenderManager::unitToClipTransform, widgetBuffer);
    }
}

} // namespace GfxPaint
