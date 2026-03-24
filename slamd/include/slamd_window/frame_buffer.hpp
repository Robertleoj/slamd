#pragma once
#include <glbinding/gl/gl.h>

namespace slamd {

class FrameBuffer {
   private:
    static const int MAX_PEEL_LAYERS = 4;

    // Resolve target (for ImGui)
    uint32_t frame_buffer_object_id = 0;
    uint32_t texture_id = 0;
    uint32_t render_buffer_object_id = 0;

    // MSAA framebuffer (opaque pass)
    uint32_t msaa_framebuffer_id = 0;
    uint32_t msaa_color_buffer_id = 0;
    uint32_t msaa_depth_buffer_id = 0;
    int samples = 4;

    // Depth peeling resources
    uint32_t peel_msaa_fbo_id = 0;
    uint32_t peel_msaa_color_id = 0;   // MSAA renderbuffer
    uint32_t peel_msaa_depth_id = 0;   // MSAA renderbuffer

    uint32_t peel_depth_fbo_id[2] = {};     // non-MSAA, for depth resolve
    uint32_t peel_depth_tex_id[2] = {};     // non-MSAA depth textures (ping-pong)

    uint32_t peel_resolve_fbo_id = 0;       // non-MSAA, for color resolve
    uint32_t peel_layer_tex_id[MAX_PEEL_LAYERS] = {};  // resolved RGBA per layer

    // Fullscreen quad for compositing
    uint32_t quad_vao_id = 0;
    uint32_t quad_vbo_id = 0;

    int current_peel_read = 0;

    size_t current_height;
    size_t current_width;

   public:
    FrameBuffer(size_t width, size_t height);
    ~FrameBuffer();

    uint32_t frame_texture();
    bool rescale(size_t width, size_t height);
    void bind();
    void unbind();
    void resolve();

    // Depth peeling interface
    void bind_peel_pass();
    uint32_t prev_peel_depth_texture() const;
    void end_peel_pass(int layer);
    void composite_peel(int num_layers);

    double aspect() const;
    size_t width() const;
    size_t height() const;

   private:
    void initialize();
};

}  // namespace slamd
