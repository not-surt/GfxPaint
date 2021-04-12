#include "workbuffermanager.h"

#include "application.h"

namespace GfxPaint {

Buffer *WorkBufferManager::fitSize(const Buffer::Format format, const QSize size) {
    FormatData &data = formats[format];
    if (size.isValid()) data.minSize = data.minSize.expandedTo(size);

    if (data.minSize.isValid() && (!data.buffer || data.buffer->width() != data.minSize.width() || data.buffer->height() != data.minSize.height())) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        FramebufferBinder framebufferBinder;
        delete data.buffer;
        data.buffer = new Buffer({qMax(data.minSize.width(), data.editSize.width()), qMax(data.minSize.height(), data.editSize.height())}, format);
    }

    if (!size.isValid()) data.minSize = data.editSize;

    return data.buffer;
}

} // namespace GfxPaint
