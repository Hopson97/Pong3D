#pragma once

#include <vector>
#include <glad/glad.h>

struct Mesh {
    std::vector<GLfloat> positions;
    std::vector<GLuint> index;
};