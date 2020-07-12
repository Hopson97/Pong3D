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
    int currentIndex = 0;
};

Mesh createCubeMesh(const glm::vec3& dimensions);
Mesh createWireCubeMesh(const glm::vec3& dimensions);
Mesh createTerrainMesh(int terrainZIndex, const glm::vec2& size, float tileSize);