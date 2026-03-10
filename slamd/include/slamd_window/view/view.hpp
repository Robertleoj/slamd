#pragma once
#include <flatb/visualizer_generated.h>
#include <slamd_window/tree/tree.hpp>
#include <slamd_window/tree_overlay.hpp>

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
    TreeOverlay tree_overlay;
};

}  // namespace slamd
