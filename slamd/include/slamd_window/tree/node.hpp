#pragma once
#include <flatb/visualizer_generated.h>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <slamd_common/id.hpp>
#include <slamd_window/geom/geometry.hpp>
#include <string>

namespace slamd {

class Node {
   public:
    static std::unique_ptr<Node> deserialize(
        const slamd::flatb::Node* node_fb,
        std::map<slamd::_id::GeometryID, std::shared_ptr<_geom::Geometry>>&
            geometries
    );
    std::map<std::string, std::unique_ptr<Node>> children;

   private:
    std::optional<glm::mat4> transform;
    std::optional<std::shared_ptr<_geom::Geometry>> object;

    mutable std::mutex transform_mutex;
    mutable std::mutex object_mutex;

   public:
    bool checked = true;
    std::optional<bool> glob_matches = std::nullopt;

    std::optional<std::shared_ptr<_geom::Geometry>> get_object() const;

    std::optional<glm::mat4> get_transform() const;

    void set_object(std::shared_ptr<_geom::Geometry> object);

    void set_transform(glm::mat4 transform);
};

}  // namespace slamd