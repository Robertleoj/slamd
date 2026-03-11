#include <slamd/geom/plane.hpp>
#include <slamd_common/gmath/serialization.hpp>

namespace slamd {
namespace geom {

Plane::Plane(
    glm::vec3 normal,
    glm::vec3 point,
    glm::vec3 color,
    float radius,
    float alpha
)
    : normal(normal),
      point(point),
      color(color),
      radius(radius),
      alpha(alpha) {}

flatbuffers::Offset<slamd::flatb::Geometry> Plane::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto normal_fb = gmath::serialize(this->normal);
    auto point_fb = gmath::serialize(this->point);
    auto color_fb = gmath::serialize(this->color);
    // auto point_fb =

    auto plane_fb = flatb::CreatePlane(
        builder,
        &normal_fb,
        &point_fb,
        &color_fb,
        this->radius,
        this->alpha
    );

    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_plane,
        plane_fb.Union()
    );
}

std::shared_ptr<Plane> plane(
    glm::vec3 normal,
    glm::vec3 point,
    glm::vec3 color,
    float radius,
    float alpha
) {
    return std::make_shared<Plane>(normal, point, color, radius, alpha);
}

}  // namespace geom
}  // namespace slamd
