#pragma once

#include <glad/glad.h>
#include <vector>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

struct Mesh;

struct BufferedMesh {
    GLuint vao;
    std::vector<GLuint> vbos;
};

BufferedMesh bufferMesh(const Mesh& mesh);

GLuint loadShaderProgram(const std::string& vShaderName, const std::string& fShaderName);
void uniformMatrix4(GLuint location, glm::mat4& matrix);
