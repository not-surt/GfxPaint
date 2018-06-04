#ifndef MODEL_H
#define MODEL_H

#include "opengl.h"

namespace GfxPaint {

class Model : protected OpenGL {
public:
    Model(const GLenum primitive, const QVector<GLsizei> &attributeSizes, const QVector<GLfloat> &vertices, const QVector<GLushort> &indices, const QVector<GLushort> &elementSizes) :
        OpenGL(true),
        primitive(primitive), elementCount(elementSizes.size()),
        vao(0), vertexBuffer(0), elementBuffer(0), indirectBuffer(0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &elementBuffer);
        glGenBuffers(1, &indirectBuffer);

        const GLsizei vertexSize = std::accumulate(attributeSizes.begin(), attributeSizes.end(), 0);
        const GLsizei vertexStride = sizeof(GLfloat) * vertexSize;
        const int vertexCount = vertices.size() / vertexSize;

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexStride, vertices.data(), GL_STATIC_DRAW);

        for (int i = 0, offset = 0; i < attributeSizes.size(); offset += attributeSizes[i], ++i) {
            glVertexAttribPointer(i, attributeSizes[i], GL_FLOAT, false, vertexStride, ((GLfloat *)0) + offset);
            glEnableVertexAttribArray(i);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
        QVector<DrawElementsIndirectCommand> commands(elementCount);
        for (int i = 0, start = 0; i < elementCount; start += elementSizes[i], ++i) {
            DrawElementsIndirectCommand &command = commands[i];
            command.count = elementSizes[i];
            command.instanceCount = 1;
            command.firstIndex = start;
            command.baseVertex = 0;
            command.baseInstance = 0;
        }
        glBufferData(GL_DRAW_INDIRECT_BUFFER, elementCount * sizeof(DrawElementsIndirectCommand), commands.data(), GL_STATIC_DRAW);
    }
    virtual ~Model() override {
        glDeleteBuffers(1, &indirectBuffer);
        glDeleteBuffers(1, &elementBuffer);
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteVertexArrays(1, &vao);
    }

    void render() {
        glBindVertexArray(vao);
        glMultiDrawElementsIndirect(primitive, GL_UNSIGNED_SHORT, nullptr, elementCount, sizeof(DrawElementsIndirectCommand));
    }

protected:
    typedef struct {
        GLuint count;
        GLuint instanceCount;
        GLuint firstIndex;
        GLuint baseVertex;
        GLuint baseInstance;
    } DrawElementsIndirectCommand;

    const GLenum primitive;
    const GLsizei elementCount;

    GLuint vao;
    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint indirectBuffer;
};

} // namespace GfxPaint

#endif // MODEL_H
