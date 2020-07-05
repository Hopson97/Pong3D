#pragma once

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

struct Mesh;

struct BufferedMesh {
    GLuint vao = 0;
    GLsizei indicesCount = 0;
    std::vector<GLuint> vbos;
};

BufferedMesh bufferMesh(const Mesh& mesh);

GLuint loadShaderProgram(const std::string& vShaderName, const std::string& fShaderName);
void uniformMatrix4(GLuint location, glm::mat4& matrix);
