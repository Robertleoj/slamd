#pragma once

#include <slamd/geom/geometry.hpp>
#include <slamd_common/data/mesh.hpp>
namespace slamd {
namespace geom {

class MonoInstanced : public Geometry {
   public:
    MonoInstanced(
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec3>& normals,
        const std::vector<uint32_t>& triangle_indices,
        const std::vector<glm::mat4>& transforms,
        const std::vector<glm::vec3>& colors
    );

    void update_transforms(const std::vector<glm::mat4>& positions);
    void update_colors(const std::vector<glm::vec3>& colors);

   private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> triangle_indices;
    std::vector<glm::mat4> transforms;
    std::vector<glm::vec3> colors;
};

}  // namespace geom
}  // namespace slamd
