#pragma once
#include <slamd/geom/geometry.hpp>
#include <slamd/geom/image.hpp>
#include <slamd/geom/poly_line.hpp>

namespace slamd {
namespace geom {

class CameraFrustum : public Geometry {
   public:
    CameraFrustum(
        glm::mat3 intrinsics_matrix,
        size_t image_width,
        size_t image_height,
        float scale = 1.0
    );

    CameraFrustum(
        glm::mat3 intrinsics_matrix,
        size_t image_width,
        size_t image_height,
        data::Image&& image,
        float scale = 1.0
    );

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

   private:
    glm::mat3 intrinsics_matrix;
    size_t image_width;
    size_t image_height;
    float scale;
    std::optional<data::Image> img = std::nullopt;
};

std::shared_ptr<CameraFrustum> camera_frustum(
    glm::mat3 intrinsics_matrix,
    size_t image_width,
    size_t image_height,
    data::Image&& image,
    float scale = 1.0
);

std::shared_ptr<CameraFrustum> camera_frustum(
    glm::mat3 intrinsics_matrix,
    size_t image_width,
    size_t image_height,
    float scale = 1.0
);

}  // namespace geom
}  // namespace slamd
