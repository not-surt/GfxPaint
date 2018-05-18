#ifndef WORKBUFFERMANAGER_H
#define WORKBUFFERMANAGER_H

#include "buffer.h"

namespace GfxPaint {

class WorkBufferManager {
public:
    WorkBufferManager() {}
    ~WorkBufferManager() {}

    Buffer *fitSize(const Buffer::Format format, const QSize size = QSize());

protected:
    struct FormatData {
        FormatData()
            : buffer(nullptr), minSize(), editSize() {}
        ~FormatData() {
            delete buffer;
        }

        Buffer *buffer;
        QSize minSize;
        QSize editSize;
    };

    QMap<Buffer::Format, FormatData> formats;
};

} // namespace GfxPaint

#endif // WORKBUFFERMANAGER_H
