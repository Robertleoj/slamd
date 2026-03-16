#pragma once
#include <flatb/visualizer_generated.h>
#include <glm/glm.hpp>
#include <slamd_common/gmath/aabb.hpp>
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/tree/node.hpp>
#include <slamd_window/tree/tree_path.hpp>

namespace slamd {

class Tree {
   public:
    uint64_t id;
    std::unique_ptr<Node> root;

   public:
    Tree(uint64_t id);
    Tree(uint64_t id, std::unique_ptr<Node>&& root);

    void
    set_object(const TreePath& path, std::shared_ptr<_geom::Geometry> object);

    void render(const glm::mat4& view, const glm::mat4& projection) const;

    static std::shared_ptr<Tree> deserialize(
        const slamd::flatb::Tree* serialized,
        std::map<slamd::_id::GeometryID, std::shared_ptr<_geom::Geometry>>&
            geometries
    );

    std::optional<slamd::gmath::AABB> bounds();
    void set_transform(const TreePath& path, const glm::mat4& transform);
    void clear(const TreePath& path);
    void mark_nodes_matching_glob(std::optional<TreePath> glob);

   private:
    std::optional<Node*> traverse(const TreePath& path);
    Node* make_path(TreePath path);

   private:
    void render_recursive(
        const Node* node,
        const glm::mat4& current_transform,
        const glm::mat4& view,
        const glm::mat4& projection
    ) const;

    std::optional<slamd::gmath::AABB>

    bounds_recursive(const Node* node, const glm::mat4& prev_transform);
};

}  // namespace slamd