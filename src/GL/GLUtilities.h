#pragma once

#include <SFML/Graphics/Image.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

struct Mesh;

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

