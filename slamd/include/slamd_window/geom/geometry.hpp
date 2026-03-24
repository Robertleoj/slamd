#pragma once
#include <flatb/geometry_generated.h>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <slamd_common/gmath/aabb.hpp>

namespace slamd {
namespace _geom {

class Geometry {
   public:
    virtual void
    render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) = 0;

    virtual bool is_transparent() const { return false; }

    virtual ~Geometry() = default;

    virtual std::optional<slamd::gmath::AABB> bounds();

    static std::shared_ptr<Geometry> deserialize(
        const slamd::flatb::Geometry* geom_fb
    );
};

}  // namespace _geom
}  // namespace slamd