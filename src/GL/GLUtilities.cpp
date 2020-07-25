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
    for (int i = 0; i < 2; i++) {
        framebuffer.textures[i].create(width, height);
        glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                       GL_TEXTURE_2D, framebuffer.textures[i].textureId(),
                                       0));
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
}
