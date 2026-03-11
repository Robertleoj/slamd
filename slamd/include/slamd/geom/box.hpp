#pragma once
#include <slamd/geom/geometry.hpp>
#include <slamd/geom/mesh.hpp>

namespace slamd {
namespace geom {

class Box : public Geometry {
   public:
    Box();

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;
};

std::shared_ptr<Box> box();

}  // namespace geom
}  // namespace slamd
