#pragma once

#include <memory>
#include <slamd/geom/geometry.hpp>

namespace slamd {
namespace geom {

/**
 * TODO: refactor to use MonoInstanced
 */
class PointCloud : public Geometry {
   public:
    PointCloud(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec3>& colors,
        const std::vector<float>& radii,
        float min_brightness
    );

    flatbuffers::Offset<slamd::flatb::Geometry> serialize(
        flatbuffers::FlatBufferBuilder& builder
    ) override;

    void update_positions(const std::vector<glm::vec3>& positions);
    void update_colors(const std::vector<glm::vec3>& colors);
    void update_radii(const std::vector<float>& radii);

   private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> colors;
    std::vector<float> radii;
    float min_brightness;
};

template <typename ColorType, typename RadiiType>
std::shared_ptr<PointCloud> point_cloud(
    const std::vector<glm::vec3>& positions,
    const ColorType& colors,
    const RadiiType& radii,
    float min_brightness = 1.0
) {
    std::vector<glm::vec3> final_colors;
    std::vector<float> final_radii;

    // Handle colors
    if constexpr (std::is_same_v<ColorType, glm::vec3>) {
        final_colors = std::vector<glm::vec3>(positions.size(), colors);
    } else {
        final_colors = colors;
    }

    // Handle radii
    if constexpr (std::is_same_v<RadiiType, float>) {
        final_radii = std::vector<float>(positions.size(), radii);
    } else {
        final_radii = radii;
    }

    auto pc = std::make_shared<PointCloud>(
        positions,
        std::move(final_colors),
        std::move(final_radii),
        min_brightness
    );

    // _global::geometries.add(pc->id, pc);
    return pc;
}

}  // namespace geom
}  // namespace slamd
