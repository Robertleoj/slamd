#include <flatb/messages_generated.h>
#include <slamd/tree/tree.hpp>
#include <slamd/view.hpp>
#include <slamd/visualizer.hpp>
#include <slamd_common/utils/serialization.hpp>

namespace slamd {
namespace _view {

std::shared_ptr<View> View::create(
    std::string name,
    std::shared_ptr<_vis::Visualizer> vis,
    std::shared_ptr<Scene> tree
) {
    auto view = std::shared_ptr<View>(new View(name, vis, tree));
    tree->attached_to.insert({view->id, view->shared_from_this()});

    return view;
}

void View::broadcast(
    std::shared_ptr<std::vector<uint8_t>> message_buffer
) {
    auto vis = this->vis.lock();
    if (!vis) {
        throw std::runtime_error("Visualizer not valid! Should never happen.");
    }

    vis->broadcast(message_buffer);
}

std::map<_id::VisualizerID, std::shared_ptr<_vis::Visualizer>>
View::find_visualizers() {
    auto vis = this->vis.lock();
    if (!vis) {
        throw std::runtime_error("Visualizer not valid! Should never happen.");
    }

    return {{vis->id, vis}};
}

flatbuffers::Offset<flatb::View> View::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto view_name_flatb = builder.CreateString(this->name);
    auto view_fb = slamd::flatb::CreateView(
        builder,
        view_name_flatb,
        this->tree->id.value
    );
    return view_fb;
}

std::shared_ptr<std::vector<uint8_t>> View::get_add_view_message() {
    flatbuffers::FlatBufferBuilder builder;
    auto view_fb = this->serialize(builder);
    auto add_view_fb = flatb::CreateAddView(builder, view_fb);
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_add_view,
        add_view_fb.Union()
    );

    builder.Finish(message_fb);

    return _utils::builder_buffer(builder);
}

std::shared_ptr<std::vector<uint8_t>> View::get_remove_view_message() {
    flatbuffers::FlatBufferBuilder builder;
    auto name_fb = builder.CreateString(this->name);
    auto remove_view_fb = flatb::CreateRemoveView(builder, name_fb);

    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_remove_view,
        remove_view_fb.Union()
    );

    builder.Finish(message_fb);

    return _utils::builder_buffer(builder);
}

View::View(
    std::string name,
    std::shared_ptr<_vis::Visualizer> vis,
    std::shared_ptr<Scene> tree
)
    : tree(tree),
      vis(vis),
      name(name) {}

View::~View() {
    this->tree->attached_to.erase(this->id);
}

}  // namespace _view
}  // namespace slamd
