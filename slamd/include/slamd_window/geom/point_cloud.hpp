#pragma once

#include <memory>
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/shaders.hpp>

namespace slamd {
namespace _geom {

/**
 * TODO: refactor to use MonoInstanced
 */
class PointCloud : public Geometry {
   public:
    PointCloud(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& colors,
        const std::vector<float>& radii,
        float min_brightness
    );
    ~PointCloud();

    PointCloud(const PointCloud&) = delete;
    PointCloud& operator=(const PointCloud&) = delete;

    void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) override;

    static std::shared_ptr<PointCloud> deserialize(
        const slamd::flatb::PointCloud* point_cloud_fb
    );

    void update_positions(const std::vector<glm::vec3>& positions);
    void update_colors(const std::vector<glm::vec3>& colors);
    void update_radii(const std::vector<float>& radii);

   private:
    // SimpleMesh mesh;

    void initialize();
    std::tuple<size_t, uint32_t, uint32_t> initialize_sphere_mesh();
    uint32_t initialize_pos_buffer();
    uint32_t initialize_radii_buffer();
    uint32_t initialize_color_buffer();
    void handle_updates();

   private:
    ShaderProgram shader;
    uint32_t vao_id = 0;
    uint32_t mesh_vbo_id = 0;
    uint32_t mesh_eab_id = 0;
    uint32_t pos_vbo_id = 0;
    uint32_t radii_vbo_id = 0;
    uint32_t colors_vbo_id = 0;
    size_t ball_vertex_count;

    std::vector<glm::vec3> positions;
    bool pending_pos_update;

    std::vector<glm::vec3> colors;
    bool pending_colors_update;

    std::vector<float> radii;
    bool pending_radii_update;

};

}  // namespace _geom
}  // namespace slamd