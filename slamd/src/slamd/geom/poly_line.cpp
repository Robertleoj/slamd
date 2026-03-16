#include <glm/gtc/matrix_transform.hpp>
#include <slamd/geom/poly_line.hpp>

#include <slamd_common/data/mesh.hpp>
#include <slamd_common/gmath/serialization.hpp>

namespace slamd {
namespace geom {

PolyLine::PolyLine(
    const std::vector<glm::vec3>& points,
    float thickness,
    const glm::vec3& color,
    float min_brightness
)
    : points(points),
      thickness(thickness),
      color(color),
      min_brightness(min_brightness) {}

flatbuffers::Offset<slamd::flatb::Geometry> PolyLine::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto color_fb = gmath::serialize(this->color);

    auto poly_line_fb = flatb::CreatePolyLine(
        builder,
        gmath::serialize_vector(builder, this->points),
        this->thickness,
        &color_fb,
        this->min_brightness
    );

    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_poly_line,
        poly_line_fb.Union()
    );
}

std::shared_ptr<PolyLine> poly_line(
    const std::vector<glm::vec3>& points,
    float thickness,
    const glm::vec3& color,
    float min_brightness
) {
    auto poly_line =
        std::make_shared<PolyLine>(points, thickness, color, min_brightness);
    // _global::geometries.add(poly_line->id, poly_line);
    return poly_line;
}

}  // namespace geom
}  // namespace slamd
