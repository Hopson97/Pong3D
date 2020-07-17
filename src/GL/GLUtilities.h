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
    void addBuffer(const std::vector<GLfloat>& data, int dims);
    void addIndexBuffer(const std::vector<GLuint>& data);

    void destroy();
};

struct Shader {
    GLuint program;
    GLuint getUniformLocation(const char* name);
    void use();
    void destroy();
};

struct Framebuffer {
    int width = 0;
    int height = 0;

    GLuint fbo = 0;
    GLuint rbo = 0;
    GLuint textures[2] = {0, 0};

    void use();
    void destroy();
};

Framebuffer makeFramebuffer(int width, int height);

BufferedMesh bufferMesh(const Mesh& mesh);
BufferedMesh bufferScreenMesh(const Mesh& mesh);

Shader loadShaderProgram(const std::string& vShaderName, const std::string& fShaderName);
void uniformMatrix4(GLuint location, glm::mat4& matrix);

void loadUniform(GLuint location, int value);
void loadUniform(GLuint location, const glm::mat4& matrix);
void loadUniform(GLuint location, const glm::vec3& vector);
