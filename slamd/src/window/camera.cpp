#include <glm/gtc/matrix_transform.hpp>
#include <slamd_window/camera.hpp>

namespace slamd {

Camera::Camera(
    double fov,
    double near_ratio,
    double far_ratio
)
    : fov(fov),
      near_ratio(near_ratio),
      far_ratio(far_ratio) {}

glm::mat4 Camera::get_projection_matrix(
    double aspect_ratio,
    double view_distance
) const {
    return glm::perspective(
        glm::radians(this->fov),
        aspect_ratio,
        near_ratio * view_distance,
        far_ratio * view_distance
    );
}

}  // namespace slamd