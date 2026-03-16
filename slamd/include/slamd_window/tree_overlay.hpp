#pragma once
#include <imgui.h>
#include <slamd_window/tree/tree.hpp>
#include <string>
#include <unordered_map>

namespace slamd {

class TreeOverlay {
   public:
    TreeOverlay();

    /// Renders the tree overlay in the upper-left of the current ImGui window.
    /// Returns true if the overlay modified tree state (checkbox toggled,
    /// filter changed).
    bool render(std::shared_ptr<Tree> tree);

   private:
    void draw_filter_field();
    void draw_tree_nodes(Node* root);
    void draw_node(
        Node* node,
        const std::string& label,
        int depth,
        bool parent_dimmed,
        TreePath& full_path
    );

    float compute_tree_content_width(Node* root) const;
    float compute_tree_content_height(Node* root) const;
    float compute_filter_row_width() const;

    char filter_buf[512] = "";
    std::unordered_map<Node*, bool> tree_open;

    bool dirty = false;

    std::shared_ptr<Tree> current_tree;
    std::optional<TreePath> filter_path;
    std::optional<std::string> filter_error;
};

}  // namespace slamd
