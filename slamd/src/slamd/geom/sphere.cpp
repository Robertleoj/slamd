#include <glm/glm.hpp>
#include <slamd/geom/sphere.hpp>

#include <slamd_common/data/mesh.hpp>
#include <slamd_common/gmath/serialization.hpp>

namespace slamd {
namespace geom {

Sphere::Sphere(
    float radius,
    glm::vec3 color
)
    : radius(radius),
      color(color) {}

flatbuffers::Offset<slamd::flatb::Geometry> Sphere::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto color_fb = gmath::serialize(this->color);
    auto sphere_fb = flatb::CreateSphere(builder, this->radius, &color_fb);
    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_sphere,
        sphere_fb.Union()
    );
}

std::shared_ptr<Sphere> sphere(
    float radius,
    glm::vec3 color
) {
    auto sphere = std::make_shared<Sphere>(radius, color);
    // _global::geometries.add(sphere->id, sphere);
    return sphere;
}

}  // namespace geom
}  // namespace slamd
