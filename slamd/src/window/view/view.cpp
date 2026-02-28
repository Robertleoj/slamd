#include <slamd_window/view/canvas_view.hpp>
#include <slamd_window/view/scene_view.hpp>
#include <slamd_window/view/view.hpp>

namespace slamd {

View::View(
    std::shared_ptr<Tree> t
)
    : tree(std::move(t)) {}

void View::mark_dirty() {
    this->_dirty = true;
}

std::unique_ptr<View> View::deserialize(
    const flatb::View* view_fb,
    std::shared_ptr<Tree> tree
) {
    std::unique_ptr<View> view;

    switch (view_fb->view_type()) {
        case (slamd::flatb::ViewType_CANVAS): {
            view = std::make_unique<CanvasView>(tree);
            break;
        }
        case (slamd::flatb::ViewType_SCENE): {
            view = std::make_unique<SceneView>(tree);
            break;
        }
        default: {
            throw std::runtime_error("Invalid geometry type");
        }
    }
    return view;
}

}  // namespace slamd