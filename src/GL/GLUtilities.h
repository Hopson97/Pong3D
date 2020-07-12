#pragma once

#include <SFML/Graphics/Image.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

struct Mesh;

struct BufferedMesh {
    GLuint vao = 0;
    GLsizei indicesCount = 0;
    std::vector<GLuint> vbos;

    void draw() const;

    void destroy();
};

struct Shader {
    GLuint program;
    GLuint getUniformLocation(const char* name);
    void use();
    void destroy();
};

BufferedMesh bufferMesh(const Mesh& mesh);

Shader loadShaderProgram(const std::string& vShaderName, const std::string& fShaderName);
void uniformMatrix4(GLuint location, glm::mat4& matrix);

void loadUniform(GLuint location, const glm::mat4& matrix);
void loadUniform(GLuint location, const glm::vec3& vector);
