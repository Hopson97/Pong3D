#include "GLUtilities.h"

#include "GLDebug.h"
#include "../Mesh.h"

BufferedMesh bufferMesh(const Mesh& mesh)
{
    BufferedMesh bufferedMesh;
    glCheck(glGenVertexArrays(1, &bufferedMesh.vao));
    glCheck(glBindVertexArray(bufferedMesh.vao));

    // Position buffer
    GLuint positionVbo;
    glCheck(glGenBuffers(1, &positionVbo));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, positionVbo));
    glCheck(glBufferData(GL_ARRAY_BUFFER,
                         mesh.positions.size() * sizeof(mesh.positions[0]),
                         mesh.positions.data(), GL_STATIC_DRAW));
    glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0));
    glCheck(glEnableVertexAttribArray(0));
    bufferedMesh.vbos.push_back(positionVbo);


    // Index buffer
    GLuint elementVbo;
    glCheck(glGenBuffers(1, &elementVbo));
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementVbo));
    glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         mesh.index.size() * sizeof(mesh.index[0]), mesh.index.data(),
                         GL_STATIC_DRAW));
    bufferedMesh.vbos.push_back(elementVbo);

    return bufferedMesh;
}
