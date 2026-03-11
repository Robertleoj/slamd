#include <spdlog/spdlog.h>
#include <slamd/geom/mono_instanced.hpp>

namespace slamd {
namespace geom {

MonoInstanced::MonoInstanced(
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals,
    const std::vector<uint32_t>& triangle_indices,
    const std::vector<glm::mat4>& transforms,
    const std::vector<glm::vec3>& colors
)
    : vertices(vertices),
      normals(normals),
      triangle_indices(triangle_indices),
      transforms(transforms),
      colors(colors) {
    if (!(vertices.size() == normals.size())) {
        throw std::invalid_argument(fmt::format(
            "number of vertices {}, number of normals {}",
            vertices.size(),
            normals.size()
        ));
    }
    if (!((transforms.size() == colors.size()))) {
        throw std::invalid_argument(fmt::format(
            "number of transforms, and colors got "
            "{} transforms and {} colors",
            transforms.size(),
            colors.size()
        ));
    }

    if ((triangle_indices.size() == 0) ||
        ((triangle_indices.size() % 3) != 0)) {
        throw std::invalid_argument("Invalid triangle indices");
    }
}

void MonoInstanced::update_transforms(
    const std::vector<glm::mat4>& transforms
) {
    this->transforms = transforms;
}

void MonoInstanced::update_colors(
    const std::vector<glm::vec3>& colors
) {
    this->colors = colors;
}

}  // namespace geom
}  // namespace slamd