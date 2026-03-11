#include <slamd_common/gmath/transforms.hpp>

namespace slamd {
namespace gmath {

glm::mat4 rot3D(
    const Angle& angle,
    glm::vec3 axis
) {
    return glm::rotate(glm::mat4(1.0), angle.rad(), axis);
}

glm::mat4 rx3D(
    const Angle& angle
) {
    return rot3D(angle, glm::vec3(1.0, 0.0, 0.0));
}

glm::mat4 ry3D(
    const Angle& angle
) {
    return rot3D(angle, glm::vec3(0.0, 1.0, 0.0));
}

glm::mat4 rz3D(
    const Angle& angle

) {
    return rot3D(angle, glm::vec3(0.0, 0.0, 1.0));
}

glm::mat4 t3D(
    const glm::vec3& amount
) {
    glm::mat4 mat(1.0);
    mat[3] = glm::vec4(amount, 1.0);
    return mat;
}

glm::mat4 tx3D(
    float amount
) {
    return t3D(glm::vec3(amount, 0.0f, 0.0f));
}

glm::mat4 ty3D(
    float amount
) {
    return t3D(glm::vec3(0.0f, amount, 0.0f));
}

glm::mat4 tz3D(
    float amount
) {
    return t3D(glm::vec3(0.0f, 0.0f, amount));
}

glm::mat4 scale_xy(
    const glm::vec2& scale_vec
) {
    return scale(glm::vec3(scale_vec, 1.0));
}

glm::mat4 scale(
    const glm::vec3& scale
) {
    return glm::scale(glm::mat4(1.0f), scale);
}

glm::mat4 scale_all(
    float amount
) {
    return scale(glm::vec3(amount, amount, amount));
}

glm::vec3 transform_point(
    glm::mat4 mat,
    glm::vec3 point
) {
    glm::vec4 homo(point, 1.0f);
    glm::vec4 transformed_homo = mat * homo;
    return glm::vec3(transformed_homo) / transformed_homo.w;
}

}  // namespace gmath
}  // namespace slamd