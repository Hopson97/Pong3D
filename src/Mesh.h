#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/common.hpp>


struct Mesh {
    std::vector<GLfloat> positions;
    std::vector<GLuint> index;
};

