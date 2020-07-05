#pragma once

#include <glad/glad.h>

#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct Mesh {
    std::vector<GLfloat> positions;
    std::vector<GLfloat> normals;
    std::vector<GLuint> indices;
};

Mesh createCubeMesh(const glm::vec3& dimensions);