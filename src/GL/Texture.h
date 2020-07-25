#pragma once

#include <glad/glad.h>

namespace glpp {
    enum class TexParam { Linear };

    class Texture2d {
      public:
        Texture2d();
        Texture2d& operator=(Texture2d&& other) noexcept;
        Texture2d(Texture2d&& other) noexcept;
        ~Texture2d();

        void create(unsigned width, unsigned height);
        void bind() const;
        GLuint textureId() const;

        Texture2d& operator=(Texture2d& other) = delete;
        Texture2d(Texture2d& other) = delete;

      private:
        GLuint m_textureId = 0;
    };
} // namespace glpp