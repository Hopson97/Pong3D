#pragma once

#include <glad/glad.h>
#include <glm/common.hpp>
#include <vector>

struct Mesh {
    std::vector<GLfloat> positions;
    std::vector<GLuint> index;
};
