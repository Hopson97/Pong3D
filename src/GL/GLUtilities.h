#pragma once

#include <glad/glad.h>
#include <vector>
#include <string>

struct Mesh {
    std::vector<GLfloat> positions;
    std::vector<GLuint> index;
};

struct BufferedMesh {
    GLuint vao;
    std::vector<GLuint> vbos;
};

BufferedMesh bufferMesh(const Mesh& mesh);

GLuint loadShaderProgram(const std::string& vShaderName, const std::string& fShaderName);