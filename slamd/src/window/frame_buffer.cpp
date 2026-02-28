#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <slamd_window/frame_buffer.hpp>
#include <stdexcept>

namespace slamd {

void FrameBuffer::initialize() {
    // === MSAA FBO ===
    gl::glGenFramebuffers(1, &msaa_framebuffer_id);
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, msaa_framebuffer_id);

    gl::glGenTextures(1, &msaa_color_buffer_id);
    gl::glBindTexture(gl::GL_TEXTURE_2D_MULTISAMPLE, msaa_color_buffer_id);
    gl::glTexImage2DMultisample(
        gl::GL_TEXTURE_2D_MULTISAMPLE,
        samples,
        gl::GL_RGB8,
        current_width,
        current_height,
        gl::GL_TRUE
    );
    gl::glFramebufferTexture2D(
        gl::GL_FRAMEBUFFER,
        gl::GL_COLOR_ATTACHMENT0,
        gl::GL_TEXTURE_2D_MULTISAMPLE,
        msaa_color_buffer_id,
        0
    );

    gl::glGenRenderbuffers(1, &msaa_depth_buffer_id);
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, msaa_depth_buffer_id);
    gl::glRenderbufferStorageMultisample(
        gl::GL_RENDERBUFFER,
        samples,
        gl::GL_DEPTH24_STENCIL8,
        current_width,
        current_height
    );
    gl::glFramebufferRenderbuffer(
        gl::GL_FRAMEBUFFER,
        gl::GL_DEPTH_STENCIL_ATTACHMENT,
        gl::GL_RENDERBUFFER,
        msaa_depth_buffer_id
    );

    if (gl::glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) !=
        gl::GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("MSAA framebuffer not complete");
    }

    // === Resolve FBO ===
    gl::glGenFramebuffers(1, &frame_buffer_object_id);
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, frame_buffer_object_id);

    gl::glGenTextures(1, &texture_id);
    gl::glBindTexture(gl::GL_TEXTURE_2D, texture_id);
    gl::glTexImage2D(
        gl::GL_TEXTURE_2D,
        0,
        gl::GL_RGB,
        current_width,
        current_height,
        0,
        gl::GL_RGB,
        gl::GL_UNSIGNED_BYTE,
        nullptr
    );
    gl::glTexParameteri(
        gl::GL_TEXTURE_2D,
        gl::GL_TEXTURE_MIN_FILTER,
        gl::GL_LINEAR
    );
    gl::glTexParameteri(
        gl::GL_TEXTURE_2D,
        gl::GL_TEXTURE_MAG_FILTER,
        gl::GL_LINEAR
    );
    gl::glFramebufferTexture2D(
        gl::GL_FRAMEBUFFER,
        gl::GL_COLOR_ATTACHMENT0,
        gl::GL_TEXTURE_2D,
        texture_id,
        0
    );

    gl::glGenRenderbuffers(1, &render_buffer_object_id);
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, render_buffer_object_id);
    gl::glRenderbufferStorage(
        gl::GL_RENDERBUFFER,
        gl::GL_DEPTH24_STENCIL8,
        current_width,
        current_height
    );
    gl::glFramebufferRenderbuffer(
        gl::GL_FRAMEBUFFER,
        gl::GL_DEPTH_STENCIL_ATTACHMENT,
        gl::GL_RENDERBUFFER,
        render_buffer_object_id
    );

    if (glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) !=
        gl::GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Resolve framebuffer not complete");
    }

    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
    gl::glBindTexture(gl::GL_TEXTURE_2D, 0);
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, 0);
}

FrameBuffer::FrameBuffer(
    size_t width,
    size_t height
)
    : current_height(height),
      current_width(width) {
    initialize();
}

FrameBuffer::~FrameBuffer() {
    gl::glDeleteFramebuffers(1, &frame_buffer_object_id);
    gl::glDeleteTextures(1, &texture_id);
    gl::glDeleteRenderbuffers(1, &render_buffer_object_id);

    gl::glDeleteFramebuffers(1, &msaa_framebuffer_id);
    gl::glDeleteTextures(1, &msaa_color_buffer_id);
    gl::glDeleteRenderbuffers(1, &msaa_depth_buffer_id);
}

uint32_t FrameBuffer::frame_texture() {
    return texture_id;
}

void FrameBuffer::bind() {
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, msaa_framebuffer_id);
    gl::glViewport(0, 0, current_width, current_height);
}

void FrameBuffer::unbind() {
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resolve() {
    gl::glBindFramebuffer(gl::GL_READ_FRAMEBUFFER, msaa_framebuffer_id);
    gl::glBindFramebuffer(gl::GL_DRAW_FRAMEBUFFER, frame_buffer_object_id);
    gl::glBlitFramebuffer(
        0,
        0,
        current_width,
        current_height,
        0,
        0,
        current_width,
        current_height,
        gl::GL_COLOR_BUFFER_BIT,
        gl::GL_NEAREST
    );
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
}

bool FrameBuffer::rescale(
    size_t width,
    size_t height
) {
    if (width == current_width && height == current_height) {
        return false;
    }

    current_width = width;
    current_height = height;

    // Realloc MSAA targets
    gl::glBindTexture(gl::GL_TEXTURE_2D_MULTISAMPLE, msaa_color_buffer_id);
    gl::glTexImage2DMultisample(
        gl::GL_TEXTURE_2D_MULTISAMPLE,
        samples,
        gl::GL_RGB8,
        width,
        height,
        gl::GL_TRUE
    );

    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, msaa_depth_buffer_id);
    gl::glRenderbufferStorageMultisample(
        gl::GL_RENDERBUFFER,
        samples,
        gl::GL_DEPTH24_STENCIL8,
        width,
        height
    );

    // Realloc resolve target
    gl::glBindTexture(gl::GL_TEXTURE_2D, texture_id);
    gl::glTexImage2D(
        gl::GL_TEXTURE_2D,
        0,
        gl::GL_RGB,
        width,
        height,
        0,
        gl::GL_RGB,
        gl::GL_UNSIGNED_BYTE,
        nullptr
    );

    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, render_buffer_object_id);
    gl::glRenderbufferStorage(
        gl::GL_RENDERBUFFER,
        gl::GL_DEPTH24_STENCIL8,
        width,
        height
    );
    return true;
}

double FrameBuffer::aspect() const {
    return static_cast<double>(current_width) /
           static_cast<double>(current_height);
}

size_t FrameBuffer::width() const {
    return current_width;
}

size_t FrameBuffer::height() const {
    return current_height;
}

}  // namespace slamd
