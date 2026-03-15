#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <slamd/constants.hpp>
#include <slamd/geom/geometry.hpp>
#include <slamd_common/data/mesh.hpp>
#include <vector>

namespace slamd {
namespace geom {

class Mesh : public Geometry {
   public:
    Mesh(
        const data::MeshData& mesh_data,
        float min_brightness = _const::default_min_brightness
    );

    Mesh(
        data::MeshData&& mesh_data,
        float min_brightness = _const::default_min_brightness
    );

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

    void update_positions(
        const std::vector<glm::vec3>& positions,
        bool recompute_normals = true
    );
    void update_colors(const std::vector<glm::vec3>& colors);
    void update_normals(const std::vector<glm::vec3>& normals);

   private:
    void
    update_normals_internal(const std::vector<glm::vec3>& normals, bool assign);

   private:
    data::MeshData mesh_data;
    float min_brightness;
};

std::shared_ptr<Mesh> mesh(const data::MeshData& mesh_data);

}  // namespace geom
}  // namespace slamd
