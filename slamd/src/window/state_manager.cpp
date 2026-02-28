#include <flatb/visualizer_generated.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <memory>
#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/gmath/stringify.hpp>
#include <slamd_window/geom/circles_2d.hpp>
#include <slamd_window/geom/mesh.hpp>
#include <slamd_window/geom/point_cloud.hpp>
#include <slamd_window/state_manager.hpp>
#include <slamd_window/view/canvas_view.hpp>
#include <slamd_window/view/scene_view.hpp>
#include <slamd_window/view/view.hpp>

namespace slamd {

StateManager::StateManager() {}

void StateManager::try_connect(
    std::string ip,
    ushort port
) {
    this->connection.emplace(ip, port);
}

void StateManager::handle_initial_state(
    const flatb::InitialState* full_state_fb
) {
    // const slamd::flatb::InitialState* full_state_fb =
    // slamd::flatb::GetInitialState(data.data());
    this->views.clear();
    this->trees.clear();
    this->geometries.clear();
    this->loaded = false;

    this->layout_path = fs::current_path() /
                        fmt::format(".{}.ini", full_state_fb->name()->str());

    auto trees_fb = full_state_fb->trees();

    for (auto geom_fb : *full_state_fb->geometries()) {
        auto geom = _geom::Geometry::deserialize(geom_fb);
        this->geometries.insert({_id::GeometryID(geom_fb->geometry_id()), geom}
        );
    }

    for (auto tree_fb : *trees_fb) {
        auto tree_id = _id::TreeID(tree_fb->id());

        this->trees.insert(
            {tree_id, Tree::deserialize(tree_fb, this->geometries)}
        );
    }

    auto views_fb = full_state_fb->views();

    for (auto view_fb : *views_fb) {
        auto tree_id = _id::TreeID(view_fb->tree_id());
        std::string view_name = view_fb->name()->str();

        auto tree = this->trees.at(tree_id);

        auto view = View::deserialize(view_fb, tree);

        this->views.insert({view_name, std::move(view)});
    }
    this->loaded = true;
    SPDLOG_INFO("loaded state");
}

void StateManager::handle_set_transform(
    const slamd::flatb::SetTransform* set_transform_fb
) {
    auto tree_id = _id::TreeID(set_transform_fb->tree_id());

    auto tree = this->trees.at(tree_id);

    TreePath path(set_transform_fb->tree_path()->str());

    auto transform = gmath::deserialize(set_transform_fb->transform());

    tree->set_transform(path, transform);
}

void StateManager::handle_set_object(
    const slamd::flatb::SetObject* set_object_fb
) {
    auto obj =
        this->geometries.at(_id::GeometryID(set_object_fb->geometry_id()));

    TreePath path(set_object_fb->tree_path()->str());
    auto tree_id = set_object_fb->tree_id();

    auto tree = this->trees.at(_id::TreeID(tree_id));

    tree->set_object(path, obj);
}

void StateManager::handle_add_geometry(
    const slamd::flatb::AddGeometry* add_geometry_fb
) {
    auto geometry = _geom::Geometry::deserialize(add_geometry_fb->geometry());
    auto geometry_id =
        _id::GeometryID(add_geometry_fb->geometry()->geometry_id());

    this->geometries.insert({geometry_id, geometry});
}

void StateManager::handle_remove_geometry(
    const slamd::flatb::RemoveGeometry* remove_geometry_fb
) {
    auto geometry_id = _id::GeometryID(remove_geometry_fb->geometry_id());
    this->geometries.erase(geometry_id);
}

void StateManager::handle_add_tree(
    const slamd::flatb::AddTree* add_tree_fb
) {
    auto tree = Tree::deserialize(add_tree_fb->tree(), this->geometries);

    this->trees.insert({_id::TreeID(add_tree_fb->tree()->id()), tree});
}

void StateManager::handle_remove_tree(
    const slamd::flatb::RemoveTree* remove_tree_fb
) {
    this->trees.erase(_id::TreeID(remove_tree_fb->tree_id()));
}

void StateManager::handle_clear_path(
    const slamd::flatb::ClearPath* clear_path_fb
) {
    auto tree = this->trees.at(_id::TreeID(clear_path_fb->tree_id()));

    TreePath path(clear_path_fb->tree_path()->str());

    tree->clear(path);
}

void StateManager::handle_remove_view(
    const slamd::flatb::RemoveView* remove_view_fb
) {
    std::string view_name = remove_view_fb->name()->str();
    SPDLOG_DEBUG("View {} removed", view_name);
    this->views.erase(view_name);
}

void StateManager::handle_add_view(
    const slamd::flatb::AddView* add_view_fb
) {
    auto view_fb = add_view_fb->view();
    auto tree_id = _id::TreeID(view_fb->tree_id());

    auto tree = this->trees.at(tree_id);

    auto view = View::deserialize(view_fb, tree);
    auto view_name = view_fb->name()->str();

    this->views.insert({view_name, std::move(view)});
}

void StateManager::handle_update_mesh_colors(
    const slamd::flatb::UpdateMeshColors* update_mesh_colors_fb
) {
    auto id = _id::GeometryID(update_mesh_colors_fb->object_id());
    auto geom = std::dynamic_pointer_cast<_geom::Mesh>(this->geometries.at(id));
    auto colors = gmath::deserialize_vector(update_mesh_colors_fb->colors());
    geom->update_colors(colors);
}
void StateManager::handle_update_mesh_positions(
    const slamd::flatb::UpdateMeshPositions* update_mesh_positions_fb
) {
    auto id = _id::GeometryID(update_mesh_positions_fb->object_id());
    auto geom = std::dynamic_pointer_cast<_geom::Mesh>(this->geometries.at(id));
    auto positions =
        gmath::deserialize_vector(update_mesh_positions_fb->positions());

    geom->update_positions(positions);
}
void StateManager::handle_update_mesh_normals(
    const slamd::flatb::UpdateMeshNormals* update_mesh_normals_fb
) {
    auto id = _id::GeometryID(update_mesh_normals_fb->object_id());
    auto geom = std::dynamic_pointer_cast<_geom::Mesh>(this->geometries.at(id));
    auto normals = gmath::deserialize_vector(update_mesh_normals_fb->normals());
    geom->update_normals(normals);
}

void StateManager::handle_update_circles2d_positions(
    const slamd::flatb::UpdateCircles2DPositions* update_fb
) {
    auto id = _id::GeometryID(update_fb->object_id());
    auto geom =
        std::dynamic_pointer_cast<_geom::Circles2D>(this->geometries.at(id));
    auto data = gmath::deserialize_vector(update_fb->positions());
    geom->update_positions(data);
}

void StateManager::handle_update_circles2d_colors(
    const slamd::flatb::UpdateCircles2DColors* update_fb
) {
    auto id = _id::GeometryID(update_fb->object_id());
    auto geom =
        std::dynamic_pointer_cast<_geom::Circles2D>(this->geometries.at(id));
    auto data = gmath::deserialize_vector(update_fb->colors());
    geom->update_colors(data);
}

void StateManager::handle_update_circles2d_radii(
    const slamd::flatb::UpdateCircles2DRadii* update_fb
) {
    auto id = _id::GeometryID(update_fb->object_id());
    auto geom =
        std::dynamic_pointer_cast<_geom::Circles2D>(this->geometries.at(id));
    auto data = gmath::deserialize_vector(update_fb->radii());
    geom->update_radii(data);
}

void StateManager::handle_update_point_cloud_positions(
    const slamd::flatb::UpdatePointCloudPositions* update_fb
) {
    auto id = _id::GeometryID(update_fb->object_id());
    auto geom =
        std::dynamic_pointer_cast<_geom::PointCloud>(this->geometries.at(id));
    auto data = gmath::deserialize_vector(update_fb->positions());
    geom->update_positions(data);
}

void StateManager::handle_update_point_cloud_colors(
    const slamd::flatb::UpdatePointCloudColors* update_fb
) {
    auto id = _id::GeometryID(update_fb->object_id());
    auto geom =
        std::dynamic_pointer_cast<_geom::PointCloud>(this->geometries.at(id));
    auto data = gmath::deserialize_vector(update_fb->colors());
    geom->update_colors(data);
}

void StateManager::handle_update_point_cloud_radii(
    const slamd::flatb::UpdatePointCloudRadii* update_fb
) {
    auto id = _id::GeometryID(update_fb->object_id());
    auto geom =
        std::dynamic_pointer_cast<_geom::PointCloud>(this->geometries.at(id));
    auto data = gmath::deserialize_vector(update_fb->radii());
    geom->update_radii(data);
}

bool StateManager::apply_updates() {
    if (!this->connection.has_value()) {
        return false;
    }

    auto& connection = this->connection.value();
    auto& message_queue = connection.messages;
    bool had_updates = false;

    while (true) {
        auto maybe_message = message_queue.try_pop();
        if (!maybe_message.has_value()) {
            return had_updates;
        }
        had_updates = true;
        auto message = std::move(maybe_message.value());

        auto message_fb = message->msg();

        switch (message_fb->message_type()) {
            case (slamd::flatb::MessageUnion_initial_state): {
                this->handle_initial_state(message_fb->message_as_initial_state(
                ));
                break;
            }
            case (slamd::flatb::MessageUnion_set_transform): {
                this->handle_set_transform(message_fb->message_as_set_transform(
                ));
                break;
            }
            case (slamd::flatb::MessageUnion_set_object): {
                this->handle_set_object(message_fb->message_as_set_object());
                break;
            }
            case (slamd::flatb::MessageUnion_add_geometry): {
                this->handle_add_geometry(message_fb->message_as_add_geometry()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_remove_geometry): {
                this->handle_remove_geometry(
                    message_fb->message_as_remove_geometry()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_add_tree): {
                this->handle_add_tree(message_fb->message_as_add_tree());
                break;
            }
            case (slamd::flatb::MessageUnion_remove_tree): {
                this->handle_remove_tree(message_fb->message_as_remove_tree());
                break;
            }
            case (flatb::MessageUnion_clear_path): {
                this->handle_clear_path(message_fb->message_as_clear_path());
                break;
            }
            case (slamd::flatb::MessageUnion_add_view): {
                this->handle_add_view(message_fb->message_as_add_view());
                break;
            }
            case (slamd::flatb::MessageUnion_remove_view): {
                this->handle_remove_view(message_fb->message_as_remove_view());
                break;
            }

            case (slamd::flatb::MessageUnion_update_mesh_colors): {
                this->handle_update_mesh_colors(
                    message_fb->message_as_update_mesh_colors()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_update_mesh_positions): {
                this->handle_update_mesh_positions(
                    message_fb->message_as_update_mesh_positions()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_update_mesh_normals): {
                this->handle_update_mesh_normals(
                    message_fb->message_as_update_mesh_normals()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_update_circles2d_colors): {
                this->handle_update_circles2d_colors(
                    message_fb->message_as_update_circles2d_colors()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_update_circles2d_positions): {
                this->handle_update_circles2d_positions(
                    message_fb->message_as_update_circles2d_positions()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_update_circles2d_radii): {
                this->handle_update_circles2d_radii(
                    message_fb->message_as_update_circles2d_radii()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_update_point_cloud_colors): {
                this->handle_update_point_cloud_colors(
                    message_fb->message_as_update_point_cloud_colors()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_update_point_cloud_positions): {
                this->handle_update_point_cloud_positions(
                    message_fb->message_as_update_point_cloud_positions()
                );
                break;
            }
            case (slamd::flatb::MessageUnion_update_point_cloud_radii): {
                this->handle_update_point_cloud_radii(
                    message_fb->message_as_update_point_cloud_radii()
                );
                break;
            }

            default: {
                SPDLOG_ERROR(
                    "Unknown message type {}",
                    static_cast<uint32_t>(message_fb->message_type())
                );
            }
        }
    }
}

}  // namespace slamd