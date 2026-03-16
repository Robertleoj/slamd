#pragma once
#include <glm/glm.hpp>

namespace slamd {
namespace gmath {

glm::vec3 get_orthogonal_vector(const glm::vec3& vec);
glm::mat4 make_frame(
    const glm::vec3& x_axis,
    const glm::vec3& y_axis,
    const glm::vec3& z_axis,
    const glm::vec3& translation
);

}  // namespace gmath
}  // namespace slamd