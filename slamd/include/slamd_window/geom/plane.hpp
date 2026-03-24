#pragma once
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/geom/mesh.hpp>

namespace slamd {
namespace _geom {
class Plane : public Geometry {
   public:
    Plane(
        const glm::vec3& normal,
        const glm::vec3& point,
        const glm::vec3& color,
        float radius,
        float alpha
    );

    void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) override;
    bool is_transparent() const override { return true; }

    static std::shared_ptr<Plane> deserialize(
        const slamd::flatb::Plane* plane_fb
    );

   private:
    std::unique_ptr<Mesh> mesh;
};

}  // namespace _geom
}  // namespace slamd