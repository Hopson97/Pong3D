#include "GLUtilities.h"

#include "../Mesh.h"
#include "GLDebug.h"
#include <exception>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

Framebuffer makeFramebuffer(int width, int height)
{
    Framebuffer framebuffer;
    framebuffer.width = width;
    framebuffer.height = height;

    // Create framebuffer
    glCheck(glGenFramebuffers(1, &framebuffer.fbo));
    framebuffer.use();

    // Create texture
    glCheck(glGenTextures(2, framebuffer.textures));

    for (int i = 0; i < 2; i++) {
        glCheck(glBindTexture(GL_TEXTURE_2D, framebuffer.textures[i]));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                             GL_UNSIGNED_BYTE, nullptr));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attatch the colours
        glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                       GL_TEXTURE_2D, framebuffer.textures[i], 0));
    }

    // Create renderbuffer
    glCheck(glGenRenderbuffers(1, &framebuffer.rbo));
    glCheck(glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.rbo));
    glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));

    glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                      GL_RENDERBUFFER, framebuffer.rbo));
    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Renderbuffer failed to be created");
    }

    return framebuffer;
}

void Framebuffer::use()
{
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
    glCheck(glViewport(0, 0, width, height));
}

void Framebuffer::destroy()
{
    glCheck(glDeleteFramebuffers(1, &fbo));
    glCheck(glDeleteRenderbuffers(1, &rbo));
    glCheck(glDeleteTextures(2, textures));
}
