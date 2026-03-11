#pragma once
#include <memory>
#include <slamd/geom/geometry.hpp>
#include <slamd_common/data/image.hpp>

namespace slamd {
namespace _geom {

class Image : public Geometry {
   public:
    Image(data::Image&& image, bool normalized = true);

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

   private:
    data::Image img;
    bool normalized;
};

}  // namespace _geom

namespace geom {
using ImagePtr = std::shared_ptr<_geom::Image>;

ImagePtr image(data::Image&& image);
}  // namespace geom

}  // namespace slamd