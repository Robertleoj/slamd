#include <flatb/messages_generated.h>
#include <slamd/constants.hpp>
#include <slamd/geom/mesh.hpp>
#include <slamd_common/assert.hpp>
#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/utils/serialization.hpp>

#include <fmt/format.h>
#include <slamd_common/data/mesh.hpp>
#include <stdexcept>

namespace slamd {
namespace geom {

void Mesh::update_positions(
    const std::vector<glm::vec3>& positions,
    bool recompute_normals
) {
    if (positions.size() != this->mesh_data.positions.size()) {
        throw std::invalid_argument(fmt::format(
            "Expected {} positions, got {}",
            this->mesh_data.positions.size(),
            positions.size()
        ));
    }

    this->mesh_data.positions = positions;

    // make the message
    flatbuffers::FlatBufferBuilder builder;

    auto positions_fb = gmath::serialize_vector(builder, positions);

    auto update_fb =
        flatb::CreateUpdateMeshPositions(builder, this->id.value, positions_fb);
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_update_mesh_positions,
        update_fb.Union()
    );

    builder.Finish(message_fb);

    this->broadcast(_utils::builder_buffer(builder));

    if (recompute_normals) {
        this->mesh_data.recompute_normals();
        this->update_normals_internal(this->mesh_data.normals, false);
    }
};

void Mesh::update_colors(
    const std::vector<glm::vec3>& colors
) {
    if (colors.size() != this->mesh_data.colors.size()) {
        throw std::invalid_argument(fmt::format(
            "Expected {} colors, got {}",
            this->mesh_data.colors.size(),
            colors.size()
        ));
    }

    this->mesh_data.colors = colors;

    // make the message
    flatbuffers::FlatBufferBuilder builder;

    auto colors_fb = gmath::serialize_vector(builder, colors);

    auto update_fb =
        flatb::CreateUpdateMeshColors(builder, this->id.value, colors_fb);
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_update_mesh_colors,
        update_fb.Union()
    );

    builder.Finish(message_fb);

    this->broadcast(_utils::builder_buffer(builder));
};

void Mesh::update_normals(
    const std::vector<glm::vec3>& normals
) {
    this->update_normals_internal(normals, true);
}

void Mesh::update_normals_internal(
    const std::vector<glm::vec3>& normals,
    bool assign
) {
    if (normals.size() != this->mesh_data.normals.size()) {
        throw std::invalid_argument(fmt::format(
            "Expected {} normals, got {}",
            this->mesh_data.normals.size(),
            normals.size()
        ));
    }

    if (assign) {
        this->mesh_data.normals = normals;
    }

    // make the message
    flatbuffers::FlatBufferBuilder builder;

    auto normals_fb = gmath::serialize_vector(builder, normals);

    auto update_fb =
        flatb::CreateUpdateMeshNormals(builder, this->id.value, normals_fb);
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_update_mesh_normals,
        update_fb.Union()
    );

    builder.Finish(message_fb);

    this->broadcast(_utils::builder_buffer(builder));
};

Mesh::Mesh(
    const data::MeshData& mesh_data,
    float min_brightness
)
    : mesh_data(mesh_data),
      min_brightness(min_brightness) {}

Mesh::Mesh(
    data::MeshData&& mesh_data,
    float min_brightness
)
    : mesh_data(std::move(mesh_data)),
      min_brightness(min_brightness) {}

flatbuffers::Offset<slamd::flatb::Geometry> Mesh::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto mesh_data_fb = this->mesh_data.serialize(builder);
    auto mesh_fb =
        flatb::CreateMesh(builder, mesh_data_fb, this->min_brightness);

    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_mesh,
        mesh_fb.Union()
    );
}

std::shared_ptr<Mesh> mesh(
    const data::MeshData& mesh_data
) {
    auto mesh = std::make_shared<Mesh>(mesh_data);
    // _global::geometries.add(mesh->id, mesh);
    return mesh;
}

}  // namespace geom
}  // namespace slamd
