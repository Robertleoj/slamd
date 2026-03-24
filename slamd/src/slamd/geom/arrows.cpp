#include <slamd/geom/arrows.hpp>
#include <slamd_common/data/mesh.hpp>
#include <slamd_common/gmath/serialization.hpp>

namespace slamd {
namespace geom {

struct ArrowMesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;
    std::vector<uint32_t> indices;
};

Arrows::Arrows(
    const std::vector<glm::vec3>& starts,
    const std::vector<glm::vec3>& ends,
    const std::vector<glm::vec3>& colors,
    float thickness
)
    : starts(starts),
      ends(ends),
      colors(colors),
      thickness(thickness) {}

flatbuffers::Offset<slamd::flatb::Geometry> Arrows::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto arrows_fb = flatb::CreateArrows(
        builder,
        gmath::serialize_vector(builder, this->starts),
        gmath::serialize_vector(builder, this->ends),
        gmath::serialize_vector(builder, this->colors),
        this->thickness
    );

    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_arrows,
        arrows_fb.Union()
    );
}

std::shared_ptr<Arrows> arrows(
    const std::vector<glm::vec3>& starts,
    const std::vector<glm::vec3>& ends,
    const std::vector<glm::vec3>& colors,
    float thickness
) {
    auto arrows = std::make_shared<Arrows>(starts, ends, colors, thickness);
    // _global::geometries.add(arrows->id, arrows);
    return arrows;
}

}  // namespace geom
}  // namespace slamd
