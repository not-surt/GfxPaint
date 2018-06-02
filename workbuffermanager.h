#ifndef WORKBUFFERMANAGER_H
#define WORKBUFFERMANAGER_H

#include "buffer.h"

namespace GfxPaint {

class WorkBufferManager {
public:
    class WorkBufferHandle {
    public:
        WorkBufferHandle() {

        }
        ~WorkBufferHandle() {

        }
        Buffer &operator*() {
            return *buffer;
        }

        Buffer *buffer;
    };
    WorkBufferManager() {}
    ~WorkBufferManager() {}

    Buffer *fitSize(const Buffer::Format format, const QSize size = QSize());

    WorkBufferHandle getWorkBuffer(const Buffer::Format &format, const QSize &size) {
        WorkBufferHandle handle;
        return handle;
    }

    void purge() {

    }


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

    QMap<Buffer::Format, QMap<QSize, Buffer *>> unused;
};

} // namespace GfxPaint

#endif // WORKBUFFERMANAGER_H
