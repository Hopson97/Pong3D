#include "GLUtilities.h"

#include "../Mesh.h"
#include "GLDebug.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

namespace {
    // For loading shaders
    GLuint compileShader(const std::string_view source, GLenum shaderType)
    {
        auto shaderID = glCheck(glCreateShader(shaderType));

        const GLchar* const shaderSourcePtr = source.data();
        const GLint shaderSourceLength = source.length();
        glCheck(glShaderSource(shaderID, 1, &shaderSourcePtr, &shaderSourceLength));
        glCheck(glCompileShader(shaderID));

        GLint logLength;

        glCheck(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength));
        if (logLength) {
            std::string infoLog(logLength, 0);
            glCheck(glGetShaderInfoLog(shaderID, logLength, nullptr, infoLog.data()));

            throw std::runtime_error(infoLog);
        }

        return shaderID;
    }

    // For loading shaders
    GLuint linkProgram(GLuint vertexShaderID, GLuint fragmentShaderID)
    {
        auto id = glCheck(glCreateProgram());

        glCheck(glAttachShader(id, vertexShaderID));
        glCheck(glAttachShader(id, fragmentShaderID));

        glCheck(glLinkProgram(id));

        glCheck(glDetachShader(id, fragmentShaderID));
        glCheck(glDetachShader(id, vertexShaderID));

        GLint logLength;

        glCheck(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength));
        if (logLength) {
            std::string infoLog(logLength, 0);
            glCheck(glGetProgramInfoLog(id, logLength, nullptr, infoLog.data()));
            throw std::runtime_error(infoLog);
        }

        return id;
    }

    // Says it on the tin
    std::string loadFile(const std::string fName)
    {
        std::ifstream inFile(fName);
        std::ostringstream stream;
        stream << inFile.rdbuf();
        return stream.str();
    }
} // namespace

BufferedMesh bufferMesh(const Mesh& mesh)
{
    BufferedMesh bufferedMesh;
    glCheck(glGenVertexArrays(1, &bufferedMesh.vao));
    glCheck(glBindVertexArray(bufferedMesh.vao));

    // Position buffer
    GLuint positionVbo;
    glCheck(glGenBuffers(1, &positionVbo));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, positionVbo));
    glCheck(glBufferData(GL_ARRAY_BUFFER,
                         mesh.positions.size() * sizeof(mesh.positions[0]),
                         mesh.positions.data(), GL_STATIC_DRAW));
    glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0));
    glCheck(glEnableVertexAttribArray(0));
    bufferedMesh.vbos.push_back(positionVbo);

    // Index buffer
    GLuint elementVbo;
    glCheck(glGenBuffers(1, &elementVbo));
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementVbo));
    glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         mesh.index.size() * sizeof(mesh.index[0]), mesh.index.data(),
                         GL_STATIC_DRAW));
    bufferedMesh.vbos.push_back(elementVbo);

    return bufferedMesh;
}

GLuint loadShaderProgram(const std::string& vShaderName, const std::string& fShaderName)
{
    std::string vertexSource = loadFile("data/" + vShaderName + "_vertex.glsl");
    std::string fragmentSource = loadFile("data/" + fShaderName + "_fragment.glsl");

    auto vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    auto fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    return linkProgram(vertexShader, fragmentShader);
}

void uniformMatrix4(GLuint location, glm::mat4& matrix)
{
    glCheck(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)));
}
