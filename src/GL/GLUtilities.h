#pragma once

#include <SFML/Graphics/Image.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

struct Mesh;

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

Shader loadShaderProgram(const std::string& vShaderName, const std::string& fShaderName);
void uniformMatrix4(GLuint location, glm::mat4& matrix);

void loadUniform(GLuint location, int value);
void loadUniform(GLuint location, const glm::mat4& matrix);
void loadUniform(GLuint location, const glm::vec3& vector);
