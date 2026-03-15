#pragma once

#include <atomic>
#include <filesystem>
#include <map>
#include <memory>
#include <slamd_common/id.hpp>
#include <slamd_window/connection.hpp>
#include <slamd_window/tree/tree.hpp>
#include <slamd_window/view/scene_view.hpp>
#include <string>

namespace slamd {

namespace fs = std::filesystem;

class StateManager {
   public:
    StateManager();

    void try_connect(std::string ip = "127.0.0.1", ushort port = 5555);

    bool apply_updates();

   private:
    void handle_initial_state(const slamd::flatb::InitialState* initial_state);
    void handle_set_transform(const slamd::flatb::SetTransform* set_transform_fb
    );
    void handle_set_object(const slamd::flatb::SetObject* set_object_fb);
    void handle_add_geometry(const slamd::flatb::AddGeometry* add_geometry_fb);
    void handle_remove_geometry(
        const slamd::flatb::RemoveGeometry* remove_geometry_fb
    );
    void handle_add_tree(const slamd::flatb::AddTree* add_tree_fb);
    void handle_remove_tree(const slamd::flatb::RemoveTree* remove_tree_fb);

    void handle_add_view(const slamd::flatb::AddView* add_view_fb);
    void handle_remove_view(const slamd::flatb::RemoveView* remove_view_fb);
    void handle_clear_path(const slamd::flatb::ClearPath* clear_path_fb);

    void handle_update_mesh_colors(
        const slamd::flatb::UpdateMeshColors* update_mesh_colors_fb
    );
    void handle_update_mesh_positions(
        const slamd::flatb::UpdateMeshPositions* update_mesh_positions_fb
    );
    void handle_update_mesh_normals(
        const slamd::flatb::UpdateMeshNormals* update_mesh_normals_fb
    );

    void handle_update_point_cloud_positions(
        const slamd::flatb::UpdatePointCloudPositions* update_fb
    );
    void handle_update_point_cloud_colors(
        const slamd::flatb::UpdatePointCloudColors* update_fb
    );
    void handle_update_point_cloud_radii(
        const slamd::flatb::UpdatePointCloudRadii* update_fb
    );

    void handle_update_spheres_positions(
        const slamd::flatb::UpdateSpheresPositions* update_fb
    );
    void handle_update_spheres_colors(
        const slamd::flatb::UpdateSpheresColors* update_fb
    );
    void handle_update_spheres_radii(
        const slamd::flatb::UpdateSpheresRadii* update_fb
    );

   public:
    std::atomic<bool> loaded = false;
    std::optional<fs::path> layout_path = std::nullopt;

    std::map<std::string, std::unique_ptr<SceneView>> views;
    std::map<_id::TreeID, std::shared_ptr<Tree>> trees;
    std::map<_id::GeometryID, std::shared_ptr<_geom::Geometry>> geometries;

   private:
    std::optional<Connection> connection;
};
}  // namespace slamd