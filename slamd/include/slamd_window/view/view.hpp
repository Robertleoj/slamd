#pragma once
#include <flatb/visualizer_generated.h>
#include <slamd_window/tree/tree.hpp>

namespace slamd {

class View {
   public:
    View(std::shared_ptr<Tree> t);
    virtual void render_to_imgui() = 0;
    virtual ~View() = default;
    static std::unique_ptr<View>
    deserialize(const flatb::View* view, std::shared_ptr<Tree> tree);

    void mark_dirty();

   public:
    bool _dirty = true;
    std::shared_ptr<Tree> tree;
    std::unordered_map<Node*, bool> tree_open;
    std::optional<std::string> visualize_glob = std::nullopt;
    char filter_buf[512] = "";
    float tree_overlay_w = 0.0f;
    float tree_overlay_h = 0.0f;
};

}  // namespace slamd