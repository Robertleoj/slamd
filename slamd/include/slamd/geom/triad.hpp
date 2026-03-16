#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <slamd/geom/geometry.hpp>

namespace slamd {
namespace geom {

class Triad : public Geometry {
   public:
    Triad(
        float scale,
        float thickness,
        std::optional<glm::mat4> pose = std::nullopt
    );

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

   private:
    float scale;
    float thickness;
    glm::mat4 pose;
};

std::shared_ptr<Triad> triad(float scale = 1.0, float thickness = 0.1);
std::shared_ptr<Triad>
triad(glm::mat4 pose, float scale = 1.0, float thickness = 0.1);

}  // namespace geom
}  // namespace slamd
