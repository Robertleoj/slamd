#include <flatb/messages_generated.h>
#include <fmt/format.h>
#include <slamd/geom/spheres.hpp>
#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/utils/serialization.hpp>

namespace slamd {
namespace geom {

Spheres::Spheres(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec3>& colors,
    const std::vector<float>& radii,
    float min_brightness
)
    : positions(positions),
      colors(colors),
      radii(radii),
      min_brightness(min_brightness) {
    if (!((positions.size() == colors.size()) && (colors.size() == radii.size())
        )) {
        throw std::invalid_argument(fmt::format(
            "number of positions, colors, and radii must be the same, got "
            "{} positions, {} colors, {} radii",
            positions.size(),
            colors.size(),
            radii.size()
        ));
    }
}

flatbuffers::Offset<slamd::flatb::Geometry> Spheres::serialize(
    flatbuffers::FlatBufferBuilder& builder
) {
    auto pos_fb = gmath::serialize_vector(builder, this->positions);
    auto colors_fb = gmath::serialize_vector(builder, this->colors);
    auto radii_fb = gmath::serialize_vector(builder, this->radii);

    auto spheres_fb = flatb::CreateSpheres(
        builder,
        pos_fb,
        colors_fb,
        radii_fb,
        this->min_brightness
    );

    return flatb::CreateGeometry(
        builder,
        this->id.value,
        flatb::GeometryUnion_spheres,
        spheres_fb.Union()
    );
}

void Spheres::update_positions(
    const std::vector<glm::vec3>& positions
) {
    if (positions.size() != this->positions.size()) {
        throw std::invalid_argument(fmt::format(
            "Expected {} positions, got {}",
            this->positions.size(),
            positions.size()
        ));
    }

    this->positions = positions;

    flatbuffers::FlatBufferBuilder builder;

    auto positions_fb = gmath::serialize_vector(builder, positions);

    auto update_fb = flatb::CreateUpdateSpheresPositions(
        builder,
        this->id.value,
        positions_fb
    );
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_update_spheres_positions,
        update_fb.Union()
    );

    builder.Finish(message_fb);

    this->broadcast(_utils::builder_buffer(builder));
};

void Spheres::update_colors(
    const std::vector<glm::vec3>& colors
) {
    if (colors.size() != this->colors.size()) {
        throw std::invalid_argument(fmt::format(
            "Expected {} colors, got {}",
            this->colors.size(),
            colors.size()
        ));
    }

    this->colors = colors;

    flatbuffers::FlatBufferBuilder builder;

    auto colors_fb = gmath::serialize_vector(builder, colors);

    auto update_fb =
        flatb::CreateUpdateSpheresColors(builder, this->id.value, colors_fb);
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_update_spheres_colors,
        update_fb.Union()
    );

    builder.Finish(message_fb);

    this->broadcast(_utils::builder_buffer(builder));
};

void Spheres::update_radii(
    const std::vector<float>& radii
) {
    if (radii.size() != this->radii.size()) {
        throw std::invalid_argument(fmt::format(
            "Expected {} radii, got {}",
            this->radii.size(),
            radii.size()
        ));
    }

    this->radii = radii;

    flatbuffers::FlatBufferBuilder builder;

    auto radii_fb = gmath::serialize_vector(builder, radii);

    auto update_fb =
        flatb::CreateUpdateSpheresRadii(builder, this->id.value, radii_fb);
    auto message_fb = flatb::CreateMessage(
        builder,
        flatb::MessageUnion_update_spheres_radii,
        update_fb.Union()
    );

    builder.Finish(message_fb);

    this->broadcast(_utils::builder_buffer(builder));
}

}  // namespace geom
}  // namespace slamd
