#include <glm/glm.hpp>
#include <slamd/geom/camera_frustum.hpp>
#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/gmath/transforms.hpp>

namespace slamd {
namespace geom {

CameraFrustum::CameraFrustum(
    glm::mat3 intrinsics_matrix,
    size_t image_width,
    size_t image_height,
    data::Image&& image,
    float scale
)
    : intrinsics_matrix(intrinsics_matrix),
      image_width(image_width),
      image_height(image_height),
      scale(scale),
      img(std::move(image))

{}

CameraFrustum::CameraFrustum(
    glm::mat3 intrinsics_matrix,
    size_t image_width,
    size_t image_height,
    float scale
)
    : intrinsics_matrix(intrinsics_matrix),
      image_width(image_width),
      image_height(image_height),
      scale(scale) {}

flatbuffers::Offset<slamd::flatb::Geometry> CameraFrustum::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    std::optional<flatbuffers::Offset<slamd::flatb::ImageData>> maybe_image_fb =
        std::nullopt;
    if (this->img.has_value()) {
        maybe_image_fb = this->img.value().serialize(builder);
    }

    auto intrinsics_fb = gmath::serialize(this->intrinsics_matrix);

    auto frustum_builder = flatb::CameraFrustumBuilder(builder);

    frustum_builder.add_intrinsics(&intrinsics_fb);

    frustum_builder.add_image_height(this->image_height);
    frustum_builder.add_image_width(this->image_width);

    frustum_builder.add_scale(this->scale);

    if (maybe_image_fb.has_value()) {
        frustum_builder.add_image(maybe_image_fb.value());
    }

    auto frustum_fb = frustum_builder.Finish();

    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_camera_frustum,
        frustum_fb.Union()
    );
}

std::shared_ptr<CameraFrustum> camera_frustum(
    glm::mat3 intrinsics_matrix,
    size_t image_width,
    size_t image_height,
    data::Image&& image,
    float scale
) {
    auto cam = std::make_shared<CameraFrustum>(
        intrinsics_matrix,
        image_width,
        image_height,
        std::move(image),
        scale
    );
    // _global::geometries.add(cam->id, cam);

    return cam;
}

std::shared_ptr<CameraFrustum> camera_frustum(
    glm::mat3 intrinsics_matrix,
    size_t image_width,
    size_t image_height,
    float scale
) {
    auto cam = std::make_shared<CameraFrustum>(
        intrinsics_matrix,
        image_width,
        image_height,
        scale
    );
    // _global::geometries.add(cam->id, cam);

    return cam;
}

}  // namespace geom
}  // namespace slamd
