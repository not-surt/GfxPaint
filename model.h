#ifndef MODEL_H
#define MODEL_H

#include "opengl.h"

namespace GfxPaint {

class Model : protected OpenGL {
public:
    Model(const GLenum primitive, const QVector<GLfloat> &vertices, const QVector<GLushort> &indices, const QVector<GLushort> &elements) :
        OpenGL(true),
        vao(0), vertexBuffer(0), elementBuffer(0), indirectBuffer(0),
        primitive(0), vertexCount(0), indexCount(0), elementCount(0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &elementBuffer);
        glGenBuffers(1, &indirectBuffer);

        const QVector<GLsizei> attributes = {2, 4};
        const GLsizei vertexSize = std::accumulate(attributes.begin(), attributes.end(), 0);
        const GLsizei stride = sizeof(GLfloat) * vertexSize;

        this->primitive = primitive;
        vertexCount = vertices.size() / vertexSize;
        indexCount = indices.size();
        elementCount = elements.size();

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * stride, vertices.data(), GL_STATIC_DRAW);

        for (int i = 0, offset = 0; i < attributes.size(); offset += attributes[i], ++i) {
            glVertexAttribPointer(i, attributes[i], GL_FLOAT, false, stride, ((GLfloat *)0) + offset);
            glEnableVertexAttribArray(i);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
        QVector<DrawElementsIndirectCommand> commands(elementCount);
        for (int i = 0, start = 0; i < elementCount; start += elements[i], ++i) {
            DrawElementsIndirectCommand &command = commands[i];
            command.count = elements[i];
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

    GLuint vao;
    GLuint vertexBuffer;
    GLuint elementBuffer;
    GLuint indirectBuffer;

    GLenum primitive;
    GLsizei vertexCount;
    GLsizei indexCount;
    GLsizei elementCount;
};

} // namespace GfxPaint

#endif // MODEL_H
