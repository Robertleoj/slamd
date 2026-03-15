#pragma once
#include <glm/glm.hpp>
#include <slamd/geom/geometry.hpp>
#include <slamd/geom/mesh.hpp>

namespace slamd {
namespace geom {

class Sphere : public Geometry {
   public:
    Sphere(float radius, glm::vec3 color);
    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

   private:
    float radius;
    glm::vec3 color;
};

std::shared_ptr<Sphere>
sphere(float radius = 1.0f, glm::vec3 color = glm::vec3(0.8, 0.2, 0.0));

}  // namespace geom
}  // namespace slamd
