#include "Mesh.h"

#include <numeric>

#include <SFML/Graphics/Image.hpp>
#include <glm/gtc/noise.hpp>
#include "../GameObjects.h"

const InstanceBatch& InstanceBatch::bind() const
{
    mesh.bind();
    return *this;
}

void InstanceBatch::buffer()
{
    mesh.buffer();
    transform_vbo_.reset();
    transform_vbo_.buffer_data(transforms);
    mesh.vao()
        .add_vertex_buffer(transform_vbo_, sizeof(glm::mat4))
        .add_instance_attribute(4, gl::Type::Float, sizeof(glm::vec4), 1, 4);
}

void InstanceBatch::draw_elements(GLenum draw_mode) const
{
    glDrawElementsInstanced(draw_mode, mesh.indices_count(), GL_UNSIGNED_INT, nullptr,
                            transforms.size());
}

void InstanceBatch::update()
{
}

// -----------------------------------
// ==== MESH GENERATION FUNCTIONS ====
// -----------------------------------

namespace
{
    void addCubeToMesh(Mesh3D& mesh, const glm::vec3& dimensions,
                       const glm::vec3& offset = {0, 0, 0})
    {
        float w = dimensions.x;
        float h = dimensions.y;
        float d = dimensions.z;

        float ox = offset.x;
        float oy = offset.y;
        float oz = offset.z;

        // Precompute corners
        glm::vec3 p000 = {ox, oy, oz};
        glm::vec3 p001 = {ox, oy, oz + d};
        glm::vec3 p010 = {ox, oy + h, oz};
        glm::vec3 p011 = {ox, oy + h, oz + d};
        glm::vec3 p100 = {ox + w, oy, oz};
        glm::vec3 p101 = {ox + w, oy, oz + d};
        glm::vec3 p110 = {ox + w, oy + h, oz};
        glm::vec3 p111 = {ox + w, oy + h, oz + d};

        auto addFace = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 normal)
        {
            GLuint startIndex = static_cast<GLuint>(mesh.vertices.size());

            mesh.vertices.push_back({a,  normal});
            mesh.vertices.push_back({b, normal});
            mesh.vertices.push_back({c,  normal});
            mesh.vertices.push_back({d, normal});

            mesh.indices.insert(mesh.indices.end(), {startIndex, startIndex + 1, startIndex + 2,
                                                     startIndex + 2, startIndex + 3, startIndex});
        };

        // Faces
        addFace(p101, p001, p011, p111, {0, 0, 1});  // Front
        addFace(p000, p100, p110, p010, {0, 0, -1}); // Back
        addFace(p001, p000, p010, p011, {-1, 0, 0}); // Left
        addFace(p100, p101, p111, p110, {1, 0, 0});  // Right
        addFace(p110, p111, p011, p010, {0, 1, 0});  // Top
        addFace(p000, p001, p101, p100, {0, -1, 0}); // Bottom
    }
} // namespace



Mesh3D generate_centered_cube_mesh(const glm::vec3& dimensions)
{
    Mesh3D mesh;

    float w = dimensions.x;
    float h = dimensions.y;
    float d = dimensions.z;

    // clang-format off
    mesh.vertices = {
        {{w, h, d},  {0.0f, 0.0f, 1.0f}},  
        {{-w, h, d}, {0.0f, 0.0f, 1.0f}},
        {{-w, -h, d},  {0.0f, 0.0f, 1.0f}},  
        {{w, -h, d},  {0.0f, 0.0f, 1.0f}},

        {{-w, h, d},  {-1.0f, 0.0f, 0.0f}}, 
        {{-w, h, -d},  {-1.0f, 0.0f, 0.0f}},
        {{-w, -h, -d},  {-1.0f, 0.0f, 0.0f}}, 
        {{-w, -h, d}, {-1.0f, 0.0f, 0.0f}},

        {{-w, h, -d},  {0.0f, 0.0f, -1.0f}}, 
        {{w, h, -d},  {0.0f, 0.0f, -1.0f}},
        {{w, -h, -d},  {0.0f, 0.0f, -1.0f}}, 
        {{-w, -h, -d},  {0.0f, 0.0f, -1.0f}},

        {{w, h, -d},  {1.0f, 0.0f, 0.0f}},  
        {{w, h, d},  {1.0f, 0.0f, 0.0f}},
        {{w, -h, d},  {1.0f, 0.0f, 0.0f}},  
        {{w, -h, -d},  {1.0f, 0.0f, 0.0f}},

        {{w, h, -d},  {0.0f, 1.0f, 0.0f}},  
        {{-w, h, -d},  {0.0f, 1.0f, 0.0f}},
        {{-w, h, d},  {0.0f, 1.0f, 0.0f}},  
        {{w, h, d},  {0.0f, 1.0f, 0.0f}},

        {{-w, -h, -d}, {0.0f, -1.0f, 0.0f}}, 
        {{w, -h, -d},  {0.0f, -1.0f, 0.0f}},
        {{w, -h, d},  {0.0f, -1.0f, 0.0f}}, 
        {{-w, -h, d},  {0.0f, -1.0f, 0.0f}},
    };
    // clang-format on

    int index = 0;
    for (int i = 0; i < 6; i++)
    {
        mesh.indices.push_back(index);
        mesh.indices.push_back(index + 1);
        mesh.indices.push_back(index + 2);
        mesh.indices.push_back(index + 2);
        mesh.indices.push_back(index + 3);
        mesh.indices.push_back(index);
        index += 4;
    }

    return mesh;
}

Mesh3D generate_terrain_mesh(int size, int edgeVertices)
{
    float fEdgeVertexCount = static_cast<float>(edgeVertices);

    Mesh3D mesh;
    for (int z = 0; z < edgeVertices; z++)
    {
        for (int x = 0; x < edgeVertices; x++)
        {
            GLfloat fz = static_cast<GLfloat>(z);
            GLfloat fx = static_cast<GLfloat>(x);

            Vertex vertex;
            vertex.position.x = fx / fEdgeVertexCount * size;
            vertex.position.y = 0.0f;
            vertex.position.z = fz / fEdgeVertexCount * size;

            
            vertex.normal = {0, 1, 0};

            mesh.vertices.push_back(vertex);
        }
    }

    for (int z = 0; z < edgeVertices - 1; z++)
    {
        for (int x = 0; x < edgeVertices - 1; x++)
        {
            int topLeft = (z * edgeVertices) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * edgeVertices) + x;
            int bottomRight = bottomLeft + 1;

            mesh.indices.push_back(topLeft);
            mesh.indices.push_back(bottomLeft);
            mesh.indices.push_back(topRight);
            mesh.indices.push_back(topRight);
            mesh.indices.push_back(bottomLeft);
            mesh.indices.push_back(bottomRight);
        }
    }

    return mesh;
}

Mesh3D createWireCubeMesh(const glm::vec3& dimensions, float wireThickness)
{
    Mesh3D cube;
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
    for (int i = 0; i < OCTAVES; i++)
    {
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

Mesh3D createTerrainMesh(int terrainZIndex, const glm::vec2& size, float tileSize)
{
    Mesh3D terrain;

    int width = static_cast<int>(size.x);
    int height = static_cast<int>(size.y);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float worldX = x * tileSize;
            float worldZ = y * tileSize;
            float heightValue =
                getNoiseAt((float)terrainZIndex, (float)x, (float)y) * 190.0f - 180.0f;

            terrain.vertices.push_back({
                .position = {worldX, heightValue, worldZ},
                .normal = {0.0f, 1.0f, 0.0f},

            });
        }
    }

    for (int y = 0; y < height - 1; y++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            GLuint i = x + y * width;

            terrain.indices.insert(terrain.indices.end(), {i, i + width, i + width + 1,
                                                           i + width + 1, i + 1, i});
        }
    }

    return terrain;
}