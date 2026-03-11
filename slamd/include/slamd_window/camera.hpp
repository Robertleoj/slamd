#pragma once

#include <glm/glm.hpp>

namespace slamd {

class Camera {
   public:
    Camera(double fov, double near_ratio, double far_ratio);
    glm::mat4 get_projection_matrix(double aspect_ratio, double view_distance) const;

   private:
    double fov;
    double near_ratio;
    double far_ratio;
};
}  // namespace slamd
