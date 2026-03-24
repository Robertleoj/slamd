#pragma once
#include <glm/glm.hpp>
#include <slamd/geom/geometry.hpp>
#include <slamd/geom/mesh.hpp>

namespace slamd {
namespace geom {

class Box : public Geometry {
   public:
    Box(glm::vec3 dims, glm::vec3 color);

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

   private:
    glm::vec3 dims;
    glm::vec3 color;
};

std::shared_ptr<Box>
box(glm::vec3 dims = glm::vec3(1.0f), glm::vec3 color = glm::vec3(0.8f, 0.2f, 0.0f));

}  // namespace geom
}  // namespace slamd
