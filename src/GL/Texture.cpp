#include "Texture.h"

#include "GLDebug.h"

namespace glpp {

    glpp::Texture2d ::Texture2d()
    {
        glCheck(glGenTextures(1, &m_textureId));
    }

    Texture2d& Texture2d ::operator=(Texture2d&& other) noexcept
    {
        m_textureId = other.m_textureId;
        other.m_textureId = 0;
        return *this;
    }

    Texture2d ::Texture2d(Texture2d&& other) noexcept
        : m_textureId{other.m_textureId}
    {
        other.m_textureId = 0;
    }

    Texture2d ::~Texture2d()
    {
        if (m_textureId) {
            glCheck(glDeleteTextures(1, &m_textureId));
        }
    }

    void Texture2d::create(unsigned width, unsigned height)
    {
        bind();
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                             GL_UNSIGNED_BYTE, nullptr));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void Texture2d::bind() const
    {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
    }

    GLuint Texture2d::textureId() const
    {
        return m_textureId;
    }

} // namespace glpp