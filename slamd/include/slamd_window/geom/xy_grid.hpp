#pragma once

#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/shaders.hpp>

namespace slamd {
namespace _geom {

class GridXYPlane : public Geometry {
   public:
    GridXYPlane(float grid_size = 10.0f);

    void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) override;

    ~GridXYPlane() override;

    GridXYPlane(const GridXYPlane&) = delete;
    GridXYPlane& operator=(const GridXYPlane&) = delete;

    void set_arcball_zoom(float zoom);

   private:
    void initialize();

   private:
    uint vao_id = 0;
    uint vbo_id = 0;
    ShaderProgram shader;
    size_t vertex_count;

    float arcball_zoom;
    float grid_size;
};

}  // namespace _geom
}  // namespace slamd