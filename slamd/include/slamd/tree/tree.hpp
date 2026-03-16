#pragma once
#include <flatb/visualizer_generated.h>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <slamd/geom/geometry.hpp>
#include <slamd/tree/tree_path.hpp>
#include <slamd_common/id.hpp>

namespace slamd {

namespace _vis {
class Visualizer;
}
namespace _view {
class View;
}

namespace geom {
class Geometry;
}

class Scene;

namespace _tree {

class Node : public std::enable_shared_from_this<Node> {
   public:
    ~Node();
    Node(Scene* scene, TreePath path);

    std::optional<glm::mat4> get_transform() const;

    void set_object(std::shared_ptr<geom::Geometry> object);

    void set_transform(glm::mat4 transform);

    Node() = delete;
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    Node& operator=(const Node&) = delete;

    // should not be in public API
    std::optional<std::shared_ptr<geom::Geometry>> get_object() const;

    flatbuffers::Offset<slamd::flatb::Node> serialize(
        flatbuffers::FlatBufferBuilder& builder
    );

    void broadcast(std::shared_ptr<std::vector<uint8_t>> message_buffer);

    std::map<_id::VisualizerID, std::shared_ptr<_vis::Visualizer>>
    find_visualizers();

   private:
    void detach_object();
    void attach_object();

   public:
    std::map<std::string, std::shared_ptr<Node>> children;
    const _id::NodeID id;

   private:
    std::optional<glm::mat4> transform;
    std::optional<std::shared_ptr<geom::Geometry>> object;

    mutable std::mutex transform_mutex;
    mutable std::mutex object_mutex;

    Scene* scene;
    const TreePath path;
};

}  // namespace _tree

class Scene {
   public:
    Scene();
    ~Scene();

    void
    set_object(const std::string& path, std::shared_ptr<geom::Geometry> object);

    void set_transform(const std::string& path, const glm::mat4& transform);

    flatbuffers::Offset<slamd::flatb::Tree> serialize(
        flatbuffers::FlatBufferBuilder& builder
    );

    void clear(const std::string& path);

    void add_all_geometries(
        std::map<_id::GeometryID, std::shared_ptr<geom::Geometry>>& initial_map
    );

    std::shared_ptr<std::vector<uint8_t>> get_add_tree_message();
    std::shared_ptr<std::vector<uint8_t>> get_remove_tree_message();

    void broadcast(std::shared_ptr<std::vector<uint8_t>> message_buffer);
    std::map<_id::VisualizerID, std::shared_ptr<_vis::Visualizer>>
    find_visualizers();

   public:
    const _id::TreeID id;
    std::map<_id::ViewID, std::weak_ptr<_view::View>> attached_to;

   private:
    std::shared_ptr<std::vector<uint8_t>> get_clear_path_message(
        const std::string& path
    );
    std::optional<_tree::Node*> traverse(const _tree::TreePath& path);
    _tree::Node* make_path(_tree::TreePath path);

    void add_all_geometries_rec(
        _tree::Node* node,
        std::map<_id::GeometryID, std::shared_ptr<geom::Geometry>>& initial_map
    );

   private:
    std::shared_ptr<_tree::Node> root;
};

std::shared_ptr<Scene> scene();

}  // namespace slamd
