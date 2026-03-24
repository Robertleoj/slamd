#include <spdlog/spdlog.h>
#include <slamd_common/gmath/transforms.hpp>
#include <slamd_window/tree/tree.hpp>

namespace slamd {

Tree::Tree(
    uint64_t id
)
    : id(id) {
    this->root = std::make_unique<Node>();
}

Tree::Tree(
    uint64_t id,
    std::unique_ptr<Node>&& root
)
    : id(id),
      root(std::move(root)) {}

void mark_glob_match_recursive(
    Node* node,
    TreePath& path,
    std::optional<TreePath>& glob
) {
    if (!glob.has_value()) {
        node->glob_matches = std::nullopt;
    } else {
        node->glob_matches = path.matches_glob(glob.value());
    }

    for (auto& [label, child] : node->children) {
        path.components.push_back(label);
        mark_glob_match_recursive(child.get(), path, glob);
        path.components.pop_back();
    }
}

void Tree::mark_nodes_matching_glob(
    std::optional<TreePath> glob
) {
    TreePath pth("/");
    mark_glob_match_recursive(this->root.get(), pth, glob);
}

std::shared_ptr<Tree> Tree::deserialize(
    const slamd::flatb::Tree* serialized,
    std::map<slamd::_id::GeometryID, std::shared_ptr<_geom::Geometry>>&
        geometries
) {
    uint64_t tree_id = serialized->id();

    auto root_fb = serialized->root();

    if (root_fb == nullptr) {
        throw std::runtime_error("Root cannot be null!");
    }

    auto root = Node::deserialize(root_fb, geometries);

    return std::make_shared<Tree>(tree_id, std::move(root));
}

void Tree::clear(
    const TreePath& path
) {
    auto current_node = this->root.get();

    for (size_t i = 0; i < path.components.size(); i++) {
        auto component = path.components[i];
        auto& children = current_node->children;
        auto it = children.find(component);

        if (it == children.end()) {
            return;
        }

        if (i == path.components.size() - 1) {
            children.erase(it);
        } else {
            current_node = it->second.get();
        }
    }

    if (path.is_root()) {
        this->root->children.clear();
    }
}

void Tree::set_object(
    const TreePath& path,
    std::shared_ptr<_geom::Geometry> object
) {
    if (path.is_root()) {
        throw std::runtime_error("Setting root object is not allowed");
    }

    Node* node = this->make_path(path);
    node->set_object(object);
}

void Tree::render(
    const glm::mat4& view,
    const glm::mat4& projection
) const {
    this->render_recursive(this->root.get(), glm::mat4(1.0), view, projection, false);
    this->render_recursive(this->root.get(), glm::mat4(1.0), view, projection, true);
}

void Tree::render_opaque(
    const glm::mat4& view,
    const glm::mat4& projection
) const {
    this->render_recursive(this->root.get(), glm::mat4(1.0), view, projection, false);
}

void Tree::render_transparent(
    const glm::mat4& view,
    const glm::mat4& projection
) const {
    this->render_recursive(this->root.get(), glm::mat4(1.0), view, projection, true);
}

void Tree::set_transform(
    const TreePath& path,
    const glm::mat4& transform
) {
    if (path.is_root()) {
        throw std::runtime_error("Setting root transform is not allowed");
    }

    Node* node = this->make_path(path);
    node->set_transform(transform);
}

std::optional<Node*> Tree::traverse(
    const TreePath& path
) {
    Node* current_node = root.get();

    for (size_t i = 0; i < path.components.size(); i++) {
        auto& component = path.components[i];

        auto child_iterator = current_node->children.find(component);

        if (child_iterator == current_node->children.end()) {
            return std::nullopt;
        }

        current_node = child_iterator->second.get();
    }

    return current_node;
}

void Tree::render_recursive(
    const Node* node,
    const glm::mat4& current_transform,
    const glm::mat4& view,
    const glm::mat4& projection,
    bool transparent_pass
) const {
    if (!node->glob_matches.has_value() && !node->checked) {
        return;
    }

    glm::mat4 next_transform = current_transform;
    auto node_transform = node->get_transform();

    if (node_transform.has_value()) {
        next_transform = current_transform * node_transform.value();
    }

    const auto node_object = node->get_object();

    if (node_object.has_value() && node->glob_matches.value_or(true)) {
        bool is_transparent = node_object.value()->is_transparent();
        if (is_transparent == transparent_pass) {
            node_object.value()->render(next_transform, view, projection);
        }
    }

    for (auto& [_, child] : node->children) {
        this->render_recursive(child.get(), next_transform, view, projection, transparent_pass);
    }
}

Node* Tree::make_path(
    TreePath path
) {
    if (path.components.size() == 0) {
        return this->root.get();
    }

    Node* current_node = root.get();

    for (size_t i = 0; i < path.components.size(); i++) {
        auto& component = path.components[i];

        auto child_iterator = current_node->children.find(component);

        if (child_iterator == current_node->children.end()) {
            // in this case, we want to create the path down to the target
            // node
            std::unique_ptr<Node> new_node = std::make_unique<Node>();
            Node* new_node_ptr = new_node.get();

            // insert the new child
            current_node->children.emplace(component, std::move(new_node));

            current_node = new_node_ptr;

        } else {
            // node exists, just use it as the current node
            current_node = child_iterator->second.get();
        }
    }

    return current_node;
}

std::optional<slamd::gmath::AABB> Tree::bounds_recursive(
    const Node* node,
    const glm::mat4& prev_transform
) {
    glm::mat4 current_transform = prev_transform;
    auto node_transform = node->get_transform();

    if (node_transform.has_value()) {
        current_transform = prev_transform * node_transform.value();
    }

    const auto node_object = node->get_object();

    std::vector<slamd::gmath::AABB> bounds;

    if (node_object.has_value()) {
        auto object_bounds = node_object.value()->bounds();
        if (object_bounds.has_value()) {
            bounds.push_back(object_bounds.value().transform(current_transform)
            );
        }
    }

    for (auto& [_, child] : node->children) {
        // this->render_recursive(child.get(), next_transform, view,
        // projection);

        auto child_transform =
            this->bounds_recursive(child.get(), current_transform);

        if (child_transform.has_value()) {
            bounds.push_back(child_transform.value());
        }
    }

    return slamd::gmath::AABB::combine(bounds);
}

std::optional<slamd::gmath::AABB> Tree::bounds() {
    return this->bounds_recursive(this->root.get(), glm::mat4(1.0f));
}

}  // namespace slamd