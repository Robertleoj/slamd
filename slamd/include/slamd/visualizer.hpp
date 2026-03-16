#pragma once
#include <flatb/geometry_generated.h>
#include <asio.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <slamd/net/client_set.hpp>
#include <slamd_common/id.hpp>
#include <thread>

namespace slamd {

namespace _view {
class View;
}

namespace geom {
class Geometry;
}

class Scene;

namespace _vis {

class Visualizer : public std::enable_shared_from_this<Visualizer> {
   public:
    Visualizer(std::string name, uint16_t port);
    ~Visualizer();

    void add_scene(std::string name, std::shared_ptr<Scene> scene);

    void delete_scene(std::string name);

    std::shared_ptr<Scene> scene(std::string name);

    void hang_forever();

    void broadcast(std::shared_ptr<std::vector<uint8_t>> message_buffer);

   private:
    void add_view(std::string name, std::shared_ptr<Scene> tree);

    std::map<_id::GeometryID, std::shared_ptr<geom::Geometry>> find_geometries(
    );

    void delete_view(std::string name);
    void remove_view_tree(std::shared_ptr<_view::View> view);

    void server_job();
    std::vector<uint8_t> get_state();
    flatbuffers::Offset<
        flatbuffers::Vector<flatbuffers::Offset<flatb::Geometry>>>
    get_geometries_fb(flatbuffers::FlatBufferBuilder& builder);

    void send_tree(std::shared_ptr<Scene> tree);

   public:
    _id::VisualizerID id;

   private:
    const uint16_t port;
    const std::string name;
    std::thread server_thread;
    std::atomic<bool> stop_requested = false;

    std::mutex view_map_mutex;

    // NOTE: Declaration order matters for destruction!
    // Members are destroyed in reverse declaration order.
    // - io_context must outlive client_set, because Connections hold
    //   asio sockets that deregister from the io_context on destruction.
    // - trees must be declared before view_name_to_view so that views are
    //   destroyed first. Otherwise, tree destruction during ~Visualizer
    //   triggers node detach which reaches back into still-alive Views
    //   whose weak_ptr<Visualizer> can no longer lock.
    asio::io_context io_context;
    std::shared_ptr<_net::ClientSet> client_set;
    std::map<_id::TreeID, std::shared_ptr<Scene>> trees;
    std::map<std::string, std::shared_ptr<_view::View>> view_name_to_view;
};

}  // namespace _vis

std::shared_ptr<_vis::Visualizer>
visualizer(std::string name, bool spawn = true, uint16_t port = 5555);

}  // namespace slamd
