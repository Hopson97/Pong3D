#include "Mesh.h"

Mesh createCubeMesh(const glm::vec3& dimensions)
{
    Mesh cube;
    float w = dimensions.x;
    float h = dimensions.y;
    float d = dimensions.z;

    // clang-format off
    cube.positions = {
        // Front of the cube
        w, h, d, 0, h, d, 0, 0, d, w, 0, d,
        // Left
        0, h, d, 0, h, 0, 0, 0, 0, 0, 0, d,
        // Back
        0, h, 0, w, h, 0, w, 0, 0, 0, 0, 0,
        // Right
        w, h, 0, w, h, d, w, 0, d, w, 0, 0,
        // Top
        w, h, 0, 0, h, 0, 0, h, d, w, h, d,
        // Bottom
        0, 0, 0, w, 0, 0, w, 0, d, 0, 0, d
    };
    cube.normals = {
        // Front of the cube
        0, 0, 1.f,    0, 0, 1.f,    0, 0, 1.f,     0, 0, 1.0f,
        // Left
        -1.f, 0, 0,  -1.f, 0, 0,   -1.f, 0, 0,    -1.f, 0, 0, 
        // Back
        0, 0, -1.f,   0, 0, -1.f,   0, 0, -1.f,    0, 0, -1.0f,
        // Right
        1.f, 0, 0,    1.f, 0, 0,    1.f, 0, 0,     1.f, 0, 0, 
        // Top
        0, 1.f, 0,    0, 1.f, 0,    0, 1.f, 0,     0, 1.f, 0,
        // Bottom
        0, -1.f, 0,   0, -1.f, 0,   0, -1.f, 0,    0, -1.f, 0,
    };
    // clang-format on

    //For each cube face, add indice
    int index = 0;
    for (int i = 0; i < 6; i++) {
        cube.indices.push_back(index);
        cube.indices.push_back(index + 1);
        cube.indices.push_back(index + 2);
        cube.indices.push_back(index + 2);
        cube.indices.push_back(index + 3);
        cube.indices.push_back(index);
        index += 4;
    }

    return cube;
}
