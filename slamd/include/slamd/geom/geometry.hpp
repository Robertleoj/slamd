#pragma once
#include <flatb/geometry_generated.h>
#include <glm/glm.hpp>
#include <map>
#include <slamd/tree/tree.hpp>
#include <slamd_common/gmath/aabb.hpp>
#include <slamd_common/id.hpp>

namespace slamd {

namespace _tree {
class Node;
}

namespace _vis {
class Visualizer;
}

namespace geom {

class Geometry {
   public:
    Geometry();
    virtual flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) = 0;

    virtual ~Geometry() = default;

    // should not be in public api
    void attach(std::shared_ptr<_tree::Node> node);
    void detach(_tree::Node* node);
    std::shared_ptr<std::vector<uint8_t>> get_add_geometry_message();
    std::shared_ptr<std::vector<uint8_t>> get_remove_geometry_message();

   private:
    std::map<_id::VisualizerID, std::shared_ptr<_vis::Visualizer>>
    find_visualizers();

   protected:
    void broadcast(std::shared_ptr<std::vector<uint8_t>> buff);

   public:
    const _id::GeometryID id;
    std::map<_id::NodeID, std::weak_ptr<_tree::Node>> attached_to;
};

}  // namespace geom
}  // namespace slamd
