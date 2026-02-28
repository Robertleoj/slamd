#pragma once

#include <chrono>
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/shaders.hpp>
#include <thread>

namespace slamd {
namespace _geom {
class ArcballIndicator : public Geometry {
   public:
    ArcballIndicator();
    ~ArcballIndicator();
    void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) override;

    void set_arcball_zoom(float zoom);
    void interact();
    bool is_animating() const;

   private:
    static glm::mat4 get_scale_mat(float scale);
    float get_alpha();
    void initialize();

   private:
    uint vao_id = 0;
    uint vbo_id = 0;
    ShaderProgram shader;

    uint vertex_count;
    float arcball_zoom;
    std::optional<std::chrono::high_resolution_clock::time_point>
        last_interacted;
};
}  // namespace _geom
}  // namespace slamd