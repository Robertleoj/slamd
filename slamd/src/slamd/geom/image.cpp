#include <slamd/geom/image.hpp>

namespace slamd {
namespace geom {

Image::Image(
    data::Image&& image,
    bool normalized
)
    : img(image),
      normalized(normalized) {}

flatbuffers::Offset<slamd::flatb::Geometry> Image::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto image_data_fb = this->img.serialize(builder);

    auto image_fb =
        flatb::CreateImage(builder, image_data_fb, this->normalized);

    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_image,
        image_fb.Union()
    );
}

std::shared_ptr<Image> image(
    data::Image&& image
) {
    auto img = std::make_shared<Image>(std::move(image), true);

    // _global::geometries.add(img->id, img);
    return img;
}

}  // namespace geom
}  // namespace slamd
