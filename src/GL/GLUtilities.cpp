#include "GLUtilities.h"

#include "../Mesh.h"
#include "GLDebug.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <exception>

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

Framebuffer makeFramebuffer(int width, int height) { 
    Framebuffer framebuffer;
    framebuffer.width = width;
    framebuffer.height = height;

    //Create framebuffer
    glCheck(glGenFramebuffers(1, &framebuffer.fbo));

    // Create texture
    glCheck(glGenTextures(1, &framebuffer.texture));
    glCheck(glActiveTexture(GL_TEXTURE0));
    glCheck(glBindTexture(GL_TEXTURE_2D, framebuffer.texture));
    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glCheck((GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    //Create renderbuffer
    glCheck(glGenRenderbuffers(1, &framebuffer.rbo));
    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.rbo));
    glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
    glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                      GL_RENDERBUFFER, framebuffer.rbo));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Renderbuffer failed to be created");
    }


    return framebuffer;
}


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

    // Normals buffer
    GLuint normalVbo;
    glCheck(glGenBuffers(1, &normalVbo));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, normalVbo));
    glCheck(glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(mesh.normals[0]),
                         mesh.normals.data(), GL_STATIC_DRAW));
    glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0));
    glCheck(glEnableVertexAttribArray(1));
    bufferedMesh.vbos.push_back(normalVbo);

    // Index buffer
    GLuint elementVbo;
    glCheck(glGenBuffers(1, &elementVbo));
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementVbo));
    glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         mesh.indices.size() * sizeof(mesh.indices[0]),
                         mesh.indices.data(), GL_STATIC_DRAW));
    bufferedMesh.vbos.push_back(elementVbo);
    bufferedMesh.indicesCount = mesh.indices.size();

    return bufferedMesh;
}

Shader loadShaderProgram(const std::string& vShaderName, const std::string& fShaderName)
{
    Shader shader;
    std::string vertexSource = loadFile("data/" + vShaderName + "_vertex.glsl");
    std::string fragmentSource = loadFile("data/" + fShaderName + "_fragment.glsl");

    auto vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    auto fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    shader.program = linkProgram(vertexShader, fragmentShader);
    return shader;
}

void uniformMatrix4(GLuint location, glm::mat4& matrix)
{
    glCheck(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)));
}

void loadUniform(GLuint location, const glm::mat4& matrix)
{
    glCheck(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)));
}

void loadUniform(GLuint location, const glm::vec3& vector)
{
    glCheck(glUniform3fv(location, 1, glm::value_ptr(vector)));
}

void BufferedMesh::draw() const
{
    glCheck(glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr));
}

void BufferedMesh::destroy()
{
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(vbos.size(), vbos.data());
        vao = 0;
        vbos.clear();
    }
}

GLuint Shader::getUniformLocation(const char* name)
{
    return glCheck(glGetUniformLocation(program, name));
}

void Shader::use() { glCheck(glUseProgram(program)); }

void Shader::destroy() { glCheck(glDeleteProgram(program)); }

void Framebuffer::use()
{
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
    glCheck(glViewport(0, 0, width, height));
}

void Framebuffer::destroy() 
{
    glCheck(glDeleteFramebuffers(1, &fbo));
    glCheck(glDeleteRenderbuffers(1, &rbo));
    glCheck(glDeleteTextures(1, &texture));
}
