#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/gmath/transforms.hpp>
#include <slamd_window/geom/triad.hpp>

namespace slamd {
namespace _geom {

std::shared_ptr<Triad> Triad::deserialize(
    const slamd::flatb::Triad* triad_fb
) {
    glm::mat4 pose = gmath::deserialize(triad_fb->pose());

    return std::make_shared<Triad>(
        triad_fb->scale(),
        triad_fb->thickness(),
        pose
    );
}

std::unique_ptr<Arrows> make_arrows(
    float thickness
) {
    glm::vec3 origin(0.0, 0.0, 0.0);

    return std::unique_ptr<Arrows>(new Arrows(
        {origin, origin, origin},
        {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}},
        {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}},
        thickness
    ));
}

Triad::Triad(
    float scale,
    float thickness,
    glm::mat4 pose
)
    : arrows(make_arrows(thickness)),
      pose(pose) {
    this->scale_transform = slamd::gmath::scale(glm::vec3(scale, scale, scale));
}

void Triad::render(
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection
) {
    this->arrows
        ->render(model * this->pose * this->scale_transform, view, projection);
}

}  // namespace _geom
}  // namespace slamd