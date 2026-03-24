#pragma once
#include <glm/glm.hpp>
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/geom/mesh.hpp>

namespace slamd {
namespace _geom {

class Box : public Geometry {
   public:
    Box(glm::vec3 dims, glm::vec3 color);
    void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) override;

    static std::shared_ptr<Box> deserialize(const slamd::flatb::Box* box_fb);

   private:
    std::unique_ptr<Mesh> box_mesh;
};

}  // namespace _geom
}  // namespace slamd
