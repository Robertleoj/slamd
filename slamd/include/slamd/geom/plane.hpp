#pragma once
#include <slamd/geom/geometry.hpp>

namespace slamd {
namespace geom {

class Plane : public Geometry {
   public:
    Plane(
        glm::vec3 normal,
        glm::vec3 point,
        glm::vec3 color,
        float radius,
        float alpha
    );

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

   private:
    glm::vec3 normal;
    glm::vec3 point;
    glm::vec3 color;
    float radius;
    float alpha;
};

std::shared_ptr<Plane> plane(
    glm::vec3 normal,
    glm::vec3 point,
    glm::vec3 color,
    float radius,
    float alpha
);

}  // namespace geom
}  // namespace slamd
