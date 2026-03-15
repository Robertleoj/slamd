#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <slamd_common/data/mesh.hpp>
#include <slamd_window/constants.hpp>
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/shaders.hpp>
#include <vector>

namespace slamd {
namespace _geom {

class Mesh : public Geometry {
   public:
    Mesh(
        const slamd::data::MeshData& mesh_data,
        float min_brightness = _const::default_min_brightness
    );

    Mesh(
        slamd::data::MeshData&& mesh_data,
        float min_brightness = _const::default_min_brightness
    );

    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    static std::shared_ptr<Mesh> deserialize(const slamd::flatb::Mesh* mesh_fb);

    void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) override;

    void update_positions(const std::vector<glm::vec3>& positions);
    void update_colors(const std::vector<glm::vec3>& colors);
    void update_normals(const std::vector<glm::vec3>& normals);

   private:
    void handle_updates();
    void initialize();

   private:
    static thread_local std::optional<ShaderProgram> shader;

    uint32_t vao_id = 0;
    uint32_t pos_vbo_id = 0;
    uint32_t color_vbo_id = 0;
    uint32_t normal_vbo_id = 0;
    uint32_t eab_id = 0;

    slamd::data::MeshData mesh_data;
    bool pos_update_pending = false;
    bool color_update_pending = false;
    bool normal_update_pending = false;

    float min_brightness;
};

}  // namespace _geom
}  // namespace slamd