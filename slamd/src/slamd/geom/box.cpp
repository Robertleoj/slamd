#include <slamd/geom/box.hpp>
#include <slamd_common/data/mesh.hpp>

namespace slamd {
namespace geom {

Box::Box() {}

flatbuffers::Offset<slamd::flatb::Geometry> Box::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto box_fb = flatb::CreateBox(builder);
    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_box,
        box_fb.Union()
    );
}

std::shared_ptr<Box> box() {
    auto box = std::make_shared<Box>();
    // _global::geometries.add(box->id, box);
    return box;
}

}  // namespace geom
}  // namespace slamd
