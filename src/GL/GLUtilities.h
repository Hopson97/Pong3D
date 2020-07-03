#pragma once

#include <glad/glad.h>
#include <vector>

struct Mesh;

struct BufferedMesh {
    GLuint vao;
    std::vector<GLuint> vbos;
};

BufferedMesh bufferMesh(const Mesh& mesh);