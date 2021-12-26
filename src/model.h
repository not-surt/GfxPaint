#ifndef MODEL_H
#define MODEL_H

#include "opengl.h"

namespace GfxPaint {

class Model : protected OpenGL {
public:
    Model(const GLenum primitive, const std::vector<GLsizei> &attributeSizes, const std::vector<GLfloat> &vertices, const std::vector<GLushort> &indices, const std::vector<GLushort> &elementSizes) :
        OpenGL(true),
        primitive(primitive), elementCount(elementSizes.size()),
        vao(0), vertexBuffer(0), elementBuffer(0), indirectBuffer(0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &elementBuffer);
        glGenBuffers(1, &indirectBuffer);

        const GLsizei vertexSize = std::accumulate(attributeSizes.begin(), attributeSizes.end(), 0);
        Q_ASSERT(vertexSize > 0);
        const GLsizei vertexStride = sizeof(GLfloat) * vertexSize;
        const int vertexCount = vertices.size() / vertexSize;

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexStride, vertices.data(), GL_STATIC_DRAW);

        for (GLsizei i = 0, offset = 0; i < (GLsizei)attributeSizes.size(); offset += attributeSizes[i], ++i) {
            glVertexAttribPointer(i, attributeSizes[i], GL_FLOAT, false, vertexStride, (GLfloat *)(offset * sizeof(GLfloat)));
            glEnableVertexAttribArray(i);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
        std::vector<DrawElementsIndirectCommand> commands(elementCount);
        for (GLushort i = 0, start = 0; i < elementCount; start += elementSizes[i], ++i) {
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
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
//        glMultiDrawElementsIndirect(primitive, GL_UNSIGNED_SHORT, nullptr, elementCount, sizeof(DrawElementsIndirectCommand));
        for (int i = 0; i < elementCount; ++i) {
            glDrawElementsIndirect(primitive, GL_UNSIGNED_SHORT, (void *)(i * sizeof(DrawElementsIndirectCommand)));
        }
    }

protected:
    struct DrawElementsIndirectCommand {
        GLuint count;
        GLuint instanceCount;
        GLuint firstIndex;
        GLuint baseVertex;
        GLuint baseInstance; // reserved, must be 0
    };

    const GLenum primitive;
    const GLsizei elementCount;

    GLuint vao;
    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint indirectBuffer;
};

} // namespace GfxPaint

#endif // MODEL_H
