#include <optional>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <slamd_window/frame_buffer.hpp>
#include <slamd_window/gen/shader_sources.hpp>
#include <slamd_window/shaders.hpp>
#include <stdexcept>

namespace slamd {

void FrameBuffer::initialize() {
    // === MSAA FBO (opaque pass) ===
    gl::glGenFramebuffers(1, &msaa_framebuffer_id);
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, msaa_framebuffer_id);

    gl::glGenTextures(1, &msaa_color_buffer_id);
    gl::glBindTexture(gl::GL_TEXTURE_2D_MULTISAMPLE, msaa_color_buffer_id);
    gl::glTexImage2DMultisample(
        gl::GL_TEXTURE_2D_MULTISAMPLE, samples, gl::GL_RGB8,
        current_width, current_height, gl::GL_TRUE
    );
    gl::glFramebufferTexture2D(
        gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0,
        gl::GL_TEXTURE_2D_MULTISAMPLE, msaa_color_buffer_id, 0
    );

    gl::glGenRenderbuffers(1, &msaa_depth_buffer_id);
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, msaa_depth_buffer_id);
    gl::glRenderbufferStorageMultisample(
        gl::GL_RENDERBUFFER, samples, gl::GL_DEPTH24_STENCIL8,
        current_width, current_height
    );
    gl::glFramebufferRenderbuffer(
        gl::GL_FRAMEBUFFER, gl::GL_DEPTH_STENCIL_ATTACHMENT,
        gl::GL_RENDERBUFFER, msaa_depth_buffer_id
    );

    if (gl::glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) !=
        gl::GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("MSAA framebuffer not complete");
    }

    // === Resolve FBO (final output for ImGui) ===
    gl::glGenFramebuffers(1, &frame_buffer_object_id);
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, frame_buffer_object_id);

    gl::glGenTextures(1, &texture_id);
    gl::glBindTexture(gl::GL_TEXTURE_2D, texture_id);
    gl::glTexImage2D(
        gl::GL_TEXTURE_2D, 0, gl::GL_RGB,
        current_width, current_height, 0,
        gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr
    );
    gl::glTexParameteri(
        gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR
    );
    gl::glTexParameteri(
        gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR
    );
    gl::glFramebufferTexture2D(
        gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0,
        gl::GL_TEXTURE_2D, texture_id, 0
    );

    gl::glGenRenderbuffers(1, &render_buffer_object_id);
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, render_buffer_object_id);
    gl::glRenderbufferStorage(
        gl::GL_RENDERBUFFER, gl::GL_DEPTH24_STENCIL8,
        current_width, current_height
    );
    gl::glFramebufferRenderbuffer(
        gl::GL_FRAMEBUFFER, gl::GL_DEPTH_STENCIL_ATTACHMENT,
        gl::GL_RENDERBUFFER, render_buffer_object_id
    );

    if (gl::glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) !=
        gl::GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Resolve framebuffer not complete");
    }

    // === Peel FBO (non-MSAA — exact depth for peeling) ===
    gl::glGenFramebuffers(1, &peel_msaa_fbo_id);
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, peel_msaa_fbo_id);

    gl::glGenRenderbuffers(1, &peel_msaa_color_id);
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, peel_msaa_color_id);
    gl::glRenderbufferStorage(
        gl::GL_RENDERBUFFER, gl::GL_RGBA8,
        current_width, current_height
    );
    gl::glFramebufferRenderbuffer(
        gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0,
        gl::GL_RENDERBUFFER, peel_msaa_color_id
    );

    gl::glGenRenderbuffers(1, &peel_msaa_depth_id);
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, peel_msaa_depth_id);
    gl::glRenderbufferStorage(
        gl::GL_RENDERBUFFER, gl::GL_DEPTH24_STENCIL8,
        current_width, current_height
    );
    gl::glFramebufferRenderbuffer(
        gl::GL_FRAMEBUFFER, gl::GL_DEPTH_STENCIL_ATTACHMENT,
        gl::GL_RENDERBUFFER, peel_msaa_depth_id
    );

    if (gl::glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) !=
        gl::GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Peel framebuffer not complete");
    }

    // === Peel depth textures (ping-pong, non-MSAA, for shader comparison) ===
    gl::glGenTextures(2, peel_depth_tex_id);
    gl::glGenFramebuffers(2, peel_depth_fbo_id);
    for (int i = 0; i < 2; i++) {
        gl::glBindTexture(gl::GL_TEXTURE_2D, peel_depth_tex_id[i]);
        gl::glTexImage2D(
            gl::GL_TEXTURE_2D, 0, gl::GL_DEPTH24_STENCIL8,
            current_width, current_height, 0,
            gl::GL_DEPTH_STENCIL, gl::GL_UNSIGNED_INT_24_8, nullptr
        );
        gl::glTexParameteri(
            gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST
        );
        gl::glTexParameteri(
            gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST
        );

        gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, peel_depth_fbo_id[i]);
        gl::glFramebufferTexture2D(
            gl::GL_FRAMEBUFFER, gl::GL_DEPTH_STENCIL_ATTACHMENT,
            gl::GL_TEXTURE_2D, peel_depth_tex_id[i], 0
        );
    }

    // === Peel layer color textures + resolve FBO ===
    gl::glGenTextures(MAX_PEEL_LAYERS, peel_layer_tex_id);
    for (int i = 0; i < MAX_PEEL_LAYERS; i++) {
        gl::glBindTexture(gl::GL_TEXTURE_2D, peel_layer_tex_id[i]);
        gl::glTexImage2D(
            gl::GL_TEXTURE_2D, 0, gl::GL_RGBA8,
            current_width, current_height, 0,
            gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr
        );
        gl::glTexParameteri(
            gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR
        );
        gl::glTexParameteri(
            gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR
        );
    }

    gl::glGenFramebuffers(1, &peel_resolve_fbo_id);
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, peel_resolve_fbo_id);
    gl::glFramebufferTexture2D(
        gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0,
        gl::GL_TEXTURE_2D, peel_layer_tex_id[0], 0
    );

    // === Fullscreen quad ===
    // clang-format off
    float quad_verts[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
    };
    // clang-format on
    gl::glGenVertexArrays(1, &quad_vao_id);
    gl::glGenBuffers(1, &quad_vbo_id);
    gl::glBindVertexArray(quad_vao_id);
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, quad_vbo_id);
    gl::glBufferData(
        gl::GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, gl::GL_STATIC_DRAW
    );
    gl::glEnableVertexAttribArray(0);
    gl::glVertexAttribPointer(
        0, 2, gl::GL_FLOAT, gl::GL_FALSE, 4 * sizeof(float), (void*)0
    );
    gl::glEnableVertexAttribArray(1);
    gl::glVertexAttribPointer(
        1, 2, gl::GL_FLOAT, gl::GL_FALSE, 4 * sizeof(float),
        (void*)(2 * sizeof(float))
    );
    gl::glBindVertexArray(0);

    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
    gl::glBindTexture(gl::GL_TEXTURE_2D, 0);
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, 0);
}

FrameBuffer::FrameBuffer(size_t width, size_t height)
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

    gl::glDeleteFramebuffers(1, &peel_msaa_fbo_id);
    gl::glDeleteRenderbuffers(1, &peel_msaa_color_id);
    gl::glDeleteRenderbuffers(1, &peel_msaa_depth_id);

    gl::glDeleteFramebuffers(2, peel_depth_fbo_id);
    gl::glDeleteTextures(2, peel_depth_tex_id);

    gl::glDeleteFramebuffers(1, &peel_resolve_fbo_id);
    gl::glDeleteTextures(MAX_PEEL_LAYERS, peel_layer_tex_id);

    gl::glDeleteVertexArrays(1, &quad_vao_id);
    gl::glDeleteBuffers(1, &quad_vbo_id);
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
        0, 0, current_width, current_height,
        0, 0, current_width, current_height,
        gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT, gl::GL_NEAREST
    );
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
}

void FrameBuffer::bind_peel_pass() {
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, peel_msaa_fbo_id);
    gl::glViewport(0, 0, current_width, current_height);

    gl::glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gl::glClearDepth(1.0);
    gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

    gl::glEnable(gl::GL_DEPTH_TEST);
    gl::glDepthMask(gl::GL_TRUE);
    gl::glDisable(gl::GL_BLEND);
}

uint32_t FrameBuffer::prev_peel_depth_texture() const {
    return peel_depth_tex_id[current_peel_read];
}

void FrameBuffer::end_peel_pass(int layer) {
    // Resolve MSAA color → layer texture
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, peel_resolve_fbo_id);
    gl::glFramebufferTexture2D(
        gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0,
        gl::GL_TEXTURE_2D, peel_layer_tex_id[layer], 0
    );

    gl::glBindFramebuffer(gl::GL_READ_FRAMEBUFFER, peel_msaa_fbo_id);
    gl::glBindFramebuffer(gl::GL_DRAW_FRAMEBUFFER, peel_resolve_fbo_id);
    gl::glBlitFramebuffer(
        0, 0, current_width, current_height,
        0, 0, current_width, current_height,
        gl::GL_COLOR_BUFFER_BIT, gl::GL_NEAREST
    );

    // Resolve MSAA depth → non-MSAA depth texture for next pass comparison
    int write_idx = 1 - current_peel_read;
    gl::glBindFramebuffer(gl::GL_DRAW_FRAMEBUFFER, peel_depth_fbo_id[write_idx]);
    gl::glBlitFramebuffer(
        0, 0, current_width, current_height,
        0, 0, current_width, current_height,
        gl::GL_DEPTH_BUFFER_BIT | gl::GL_STENCIL_BUFFER_BIT, gl::GL_NEAREST
    );

    current_peel_read = write_idx;
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
}

static thread_local std::optional<ShaderProgram> composite_shader;

void FrameBuffer::composite_peel(int num_layers) {
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, frame_buffer_object_id);
    gl::glViewport(0, 0, current_width, current_height);

    gl::glDisable(gl::GL_DEPTH_TEST);
    gl::glEnable(gl::GL_BLEND);
    gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);

    if (!composite_shader.has_value()) {
        composite_shader.emplace(
            shader_source::peel_composite::vert,
            shader_source::peel_composite::frag
        );
    }
    auto& shader = composite_shader.value();
    shader.use();
    shader.set_uniform("layer_texture", 0);

    gl::glBindVertexArray(quad_vao_id);

    // Back-to-front: furthest layer first
    for (int i = num_layers - 1; i >= 0; i--) {
        gl::glActiveTexture(gl::GL_TEXTURE0);
        gl::glBindTexture(gl::GL_TEXTURE_2D, peel_layer_tex_id[i]);
        gl::glDrawArrays(gl::GL_TRIANGLES, 0, 6);
    }

    gl::glBindVertexArray(0);
    gl::glEnable(gl::GL_DEPTH_TEST);
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);
}

bool FrameBuffer::rescale(size_t width, size_t height) {
    if (width == current_width && height == current_height) {
        return false;
    }

    current_width = width;
    current_height = height;

    // MSAA targets
    gl::glBindTexture(gl::GL_TEXTURE_2D_MULTISAMPLE, msaa_color_buffer_id);
    gl::glTexImage2DMultisample(
        gl::GL_TEXTURE_2D_MULTISAMPLE, samples, gl::GL_RGB8,
        width, height, gl::GL_TRUE
    );
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, msaa_depth_buffer_id);
    gl::glRenderbufferStorageMultisample(
        gl::GL_RENDERBUFFER, samples, gl::GL_DEPTH24_STENCIL8, width, height
    );

    // Resolve target
    gl::glBindTexture(gl::GL_TEXTURE_2D, texture_id);
    gl::glTexImage2D(
        gl::GL_TEXTURE_2D, 0, gl::GL_RGB, width, height, 0,
        gl::GL_RGB, gl::GL_UNSIGNED_BYTE, nullptr
    );
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, render_buffer_object_id);
    gl::glRenderbufferStorage(
        gl::GL_RENDERBUFFER, gl::GL_DEPTH24_STENCIL8, width, height
    );

    // Peel targets
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, peel_msaa_color_id);
    gl::glRenderbufferStorage(
        gl::GL_RENDERBUFFER, gl::GL_RGBA8, width, height
    );
    gl::glBindRenderbuffer(gl::GL_RENDERBUFFER, peel_msaa_depth_id);
    gl::glRenderbufferStorage(
        gl::GL_RENDERBUFFER, gl::GL_DEPTH24_STENCIL8, width, height
    );

    // Peel depth textures
    for (int i = 0; i < 2; i++) {
        gl::glBindTexture(gl::GL_TEXTURE_2D, peel_depth_tex_id[i]);
        gl::glTexImage2D(
            gl::GL_TEXTURE_2D, 0, gl::GL_DEPTH24_STENCIL8,
            width, height, 0,
            gl::GL_DEPTH_STENCIL, gl::GL_UNSIGNED_INT_24_8, nullptr
        );
    }

    // Peel layer textures
    for (int i = 0; i < MAX_PEEL_LAYERS; i++) {
        gl::glBindTexture(gl::GL_TEXTURE_2D, peel_layer_tex_id[i]);
        gl::glTexImage2D(
            gl::GL_TEXTURE_2D, 0, gl::GL_RGBA8,
            width, height, 0,
            gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, nullptr
        );
    }

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
