#pragma once

#include <glm/glm.hpp>
#include <slamd/constants.hpp>
#include <slamd/geom/geometry.hpp>
#include <slamd/geom/mesh.hpp>
#include <vector>

namespace slamd {
namespace geom {

class PolyLine : public Geometry {
   public:
    PolyLine(
        const std::vector<glm::vec3>& points,
        float thickness,
        const glm::vec3& color,
        float min_brightness
    );

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

   private:
    std::vector<glm::vec3> points;
    float thickness;
    glm::vec3 color;
    float min_brightness;
};

std::shared_ptr<PolyLine> poly_line(
    const std::vector<glm::vec3>& points,
    float thickness,
    const glm::vec3& color,
    float min_brightness = 1.0
);

}  // namespace geom
}  // namespace slamd
