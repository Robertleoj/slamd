#pragma once
#include <asio.hpp>
#include <flatb/geometry_generated.h>
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

namespace _geom {
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
    void add_view(
        std::string name,
        std::shared_ptr<Scene> tree
    );

    std::map<_id::GeometryID, std::shared_ptr<_geom::Geometry>> find_geometries(
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

    std::map<std::string, std::shared_ptr<_view::View>> view_name_to_view;
    std::map<_id::TreeID, std::shared_ptr<Scene>> trees;
    std::shared_ptr<_net::ClientSet> client_set;
};

}  // namespace _vis

using VisualizerPtr = std::shared_ptr<_vis::Visualizer>;

VisualizerPtr
visualizer(std::string name, bool spawn = true, uint16_t port = 5555);

}  // namespace slamd
