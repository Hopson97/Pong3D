#include "Mesh.h"

#include "Maths.h"
#include <glm/gtc/noise.hpp>
#include <iostream>

#include "GameObjects.h"

namespace {
    void addCubeToMesh(Mesh& mesh, const glm::vec3& dimensions,
                       const glm::vec3& offset = {0, 0, 0})
    {
        float w = dimensions.x + offset.x;
        float h = dimensions.y + offset.y;
        float d = dimensions.z + offset.z;

        float ox = offset.x;
        float oy = offset.y;
        float oz = offset.z;

        // clang-format off
        mesh.positions.insert(mesh.positions.end(), {
            // Front of the cube
            w, h, d,    ox, h, d,   ox, oy, d,  w, oy, d,
            // Left
            ox, h, d,   ox, h, oz,  ox, oy, oz, ox, oy, d,
            // Back
            ox, h, oz,  w, h, oz,   w, oy, oz,  ox, oy, oz,
            // Right
            w, h, oz,   w, h, d,    w, oy, d,   w, oy, oz,
            // Top
            w, h, oz,   ox, h, oz,  ox, h, d,   w, h, d,
            // Bottom
            ox, oy, oz, w, oy, oz,  w, oy, d,   ox, oy, d
        });
    
        mesh.normals.insert(mesh.normals.end(), {
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
        });
        // clang-format on

        // For each cube face, add indice
        for (int i = 0; i < 6; i++) {
            mesh.indices.push_back(mesh.currentIndex);
            mesh.indices.push_back(mesh.currentIndex + 1);
            mesh.indices.push_back(mesh.currentIndex + 2);
            mesh.indices.push_back(mesh.currentIndex + 2);
            mesh.indices.push_back(mesh.currentIndex + 3);
            mesh.indices.push_back(mesh.currentIndex);
            mesh.currentIndex += 4;
        }
    }
} // namespace

Mesh createScreenMesh()
{
    Mesh quad;
    // clang-format off
    quad.positions = {
        -1.f, -1.f,
        1.f, -1.f,
        1.f, 1.f,
        -1.f, 1.f
    };

    quad.textureCoords = {
        0.0f, 0.0f, 
        1.0f, 0.0f, 
        1.0f, 1.0f, 
        0.0f, 1.0f
    };

    quad.indices = {
        0, 1, 2, 
        2, 3, 0
    };

    // clang-format on
    return quad;
}

Mesh createCubeMesh(const glm::vec3& dimensions)
{
    Mesh cube;
    addCubeToMesh(cube, dimensions);
    return cube;
}

Mesh createWireCubeMesh(const glm::vec3& dimensions, float wireThickness)
{
    Mesh cube;
    float w = dimensions.x;
    float h = dimensions.y;
    float d = dimensions.z;
    // Front
    addCubeToMesh(cube, {w, wireThickness, wireThickness});
    addCubeToMesh(cube, {w, wireThickness, wireThickness}, {0, h, 0});
    addCubeToMesh(cube, {wireThickness, h, wireThickness});
    addCubeToMesh(cube, {wireThickness, h, wireThickness}, {w, 0, 0});

    // Back
    addCubeToMesh(cube, {w, wireThickness, wireThickness}, {0, 0, d});
    addCubeToMesh(cube, {w, wireThickness, wireThickness}, {0, h, d});
    addCubeToMesh(cube, {wireThickness, h, wireThickness}, {0, 0, d});
    addCubeToMesh(cube, {wireThickness, h, wireThickness}, {w, 0, d});

    // Right
    addCubeToMesh(cube, {wireThickness, wireThickness, d}, {0, h, 0});
    addCubeToMesh(cube, {wireThickness, wireThickness, d});

    // Left
    addCubeToMesh(cube, {wireThickness, wireThickness, d}, {w, h, 0});
    addCubeToMesh(cube, {wireThickness, wireThickness, d}, {w, 0, 0});

    return cube;
}

float getNoiseAt(float z, float vx, float vz)
{
    const float ROUGH = 1.8f;
    const float SMOOTH = 11.0f;
    const int OCTAVES = 5;

    float vertexX = vx;
    float vertexZ = vz + z * (TERRAIN_HEIGHT - 1);

    float value = 0;
    float acc = 0;
    for (int i = 0; i < OCTAVES; i++) {
        float freq = glm::pow(2.0f, i);
        float amps = glm::pow(ROUGH, i);

        float x = vertexX * freq / SMOOTH;
        float z = vertexZ * freq / SMOOTH;

        float noiseValue = glm::simplex(glm::vec2{x, z});
        noiseValue = (noiseValue + 1.0f) / 2.0f;
        value += noiseValue * amps;
        acc += amps;
    }
    return value / acc;
}

Mesh createTerrainMesh(int terrainZIndex, const glm::vec2& size, float tileSize)
{
    Mesh terrian;
    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            terrian.positions.push_back(x * tileSize);
            terrian.positions.push_back(
                getNoiseAt((float)terrainZIndex, (float)x, (float)y) * 190.0f - 180.0f);
            terrian.positions.push_back(y * tileSize);

            terrian.normals.push_back(0);
            terrian.normals.push_back(1);
            terrian.normals.push_back(0);
        }
    }
    for (int y = 0; y < size.y - 1; y++) {
        for (int x = 0; x < size.x - 1; x++) {
            GLuint index = x + y * static_cast<int>(size.x);
            terrian.indices.push_back(index);
            terrian.indices.push_back(index + static_cast<int>(size.x));
            terrian.indices.push_back(index + static_cast<int>(size.x) + 1);
            terrian.indices.push_back(index + static_cast<int>(size.x) + 1);
            terrian.indices.push_back(index + 1);
            terrian.indices.push_back(index);
        }
    }
    return terrian;
}