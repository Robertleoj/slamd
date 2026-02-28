#pragma once
#include <glbinding/gl/gl.h>

namespace slamd {

class FrameBuffer {
   private:
    // Resolve target (for ImGui)
    uint32_t frame_buffer_object_id = 0;
    uint32_t texture_id = 0;
    uint32_t render_buffer_object_id = 0;

    // MSAA framebuffer
    uint32_t msaa_framebuffer_id = 0;
    uint32_t msaa_color_buffer_id = 0;
    uint32_t msaa_depth_buffer_id = 0;
    int samples = 4;  // Multisample level (can tweak later)

    size_t current_height;
    size_t current_width;

   public:
    FrameBuffer(size_t width, size_t height);
    ~FrameBuffer();

    uint32_t frame_texture();  // Resolved texture ID
    bool rescale(size_t width, size_t height);
    void bind();     // Bind MSAA FBO for rendering
    void unbind();   // Unbind
    void resolve();  // Blit MSAA → resolved texture

    double aspect() const;
    size_t width() const;
    size_t height() const;

   private:
    void initialize();
};

}  // namespace slamd
