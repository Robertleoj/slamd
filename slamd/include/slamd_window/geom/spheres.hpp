#pragma once
#include <slamd_common/data/mesh.hpp>
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/geom/mono_instanced.hpp>

namespace slamd {
namespace _geom {

class Spheres : public Geometry {
   public:
    Spheres(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& colors,
        const std::vector<float>& radii,
        float min_brightness
    );

    static std::shared_ptr<Spheres> deserialize(
        const slamd::flatb::Spheres* spheres_fb
    );

    void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) override;

    void update_positions(const std::vector<glm::vec3>& positions);
    void update_colors(const std::vector<glm::vec3>& colors);
    void update_radii(const std::vector<float>& radii);

   private:
    static std::unique_ptr<MonoInstanced> make_mono_instanced(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& colors,
        const std::vector<float>& radii
    );

    static std::vector<glm::mat4> make_transforms(
        const std::vector<glm::vec3>& positions,
        const std::vector<float>& radii
    );

    void handle_updates();

   private:
    std::unique_ptr<MonoInstanced> mono;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> colors;
    std::vector<float> radii;
    float min_brightness;

    bool pending_trans_update = false;
    bool pending_color_update = false;
};

}  // namespace _geom
}  // namespace slamd
