#include <slamd/geom/box.hpp>
#include <slamd_common/data/mesh.hpp>
#include <slamd_common/gmath/serialization.hpp>

namespace slamd {
namespace geom {

Box::Box(glm::vec3 dims, glm::vec3 color)
    : dims(dims),
      color(color) {}

flatbuffers::Offset<slamd::flatb::Geometry> Box::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto dims_fb = gmath::serialize(this->dims);
    auto color_fb = gmath::serialize(this->color);
    auto box_fb = flatb::CreateBox(builder, &dims_fb, &color_fb);
    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_box,
        box_fb.Union()
    );
}

std::shared_ptr<Box> box(glm::vec3 dims, glm::vec3 color) {
    return std::make_shared<Box>(dims, color);
}

}  // namespace geom
}  // namespace slamd
