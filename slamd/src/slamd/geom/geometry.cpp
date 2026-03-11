#include <flatb/messages_generated.h>
#include <slamd/geom/geometry.hpp>
#include <slamd/visualizer.hpp>
#include <slamd_common/utils/serialization.hpp>

namespace slamd {
namespace _geom {

Geometry::Geometry()
    : id(_id::GeometryID::next()) {}

std::shared_ptr<std::vector<uint8_t>> Geometry::get_add_geometry_message() {
    // tell the visualizer to register this object
    flatbuffers::FlatBufferBuilder builder;
    auto this_fb = this->serialize(builder);
    auto add_geometry_fb = flatb::CreateAddGeometry(builder, this_fb);
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_add_geometry,
        add_geometry_fb.Union()
    );

    builder.Finish(message_fb);
    return _utils::builder_buffer(builder);
}
std::shared_ptr<std::vector<uint8_t>> Geometry::get_remove_geometry_message() {
    flatbuffers::FlatBufferBuilder builder;
    auto remove_geometry_fb =
        flatb::CreateRemoveGeometry(builder, this->id.value);
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_remove_geometry,
        remove_geometry_fb.Union()
    );

    builder.Finish(message_fb);

    return _utils::builder_buffer(builder);
}

void Geometry::attach(
    std::shared_ptr<_tree::Node> node
) {
    auto vis_before = this->find_visualizers();
    auto node_vis = node->find_visualizers();

    for (auto& [key, vis] : node_vis) {
        if (vis_before.find(key) == vis_before.end()) {
            vis->broadcast(this->get_add_geometry_message());
        }
    }

    this->attached_to.insert({node->id, node});
}

void Geometry::detach(
    _tree::Node* node
) {
    auto node_vis = node->find_visualizers();
    this->attached_to.erase(node->id);
    auto vis_after = this->find_visualizers();

    for (auto& [key, vis] : node_vis) {
        if (vis_after.find(key) == vis_after.end()) {
            // tell the visualizer to register this object
            vis->broadcast(this->get_remove_geometry_message());
        }
    }
}

void Geometry::broadcast(
    std::shared_ptr<std::vector<uint8_t>> buff
) {
    auto visualizers = this->find_visualizers();

    for (auto& [key, vis] : visualizers) {
        vis->broadcast(buff);
    }
}

std::map<_id::VisualizerID, std::shared_ptr<_vis::Visualizer>>
Geometry::find_visualizers() {
    std::map<_id::VisualizerID, std::shared_ptr<_vis::Visualizer>> map;

    for (auto it = this->attached_to.begin(); it != this->attached_to.end();) {
        if (auto node = it->second.lock()) {
            auto node_vis_map = node->find_visualizers();
            map.insert(node_vis_map.begin(), node_vis_map.end());
            it++;
        } else {
            it = attached_to.erase(it);
        }
    }

    return map;
}

}  // namespace _geom
}  // namespace slamd