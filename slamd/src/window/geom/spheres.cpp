#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/gmath/transforms.hpp>
#include <slamd_common/utils/mesh.hpp>
#include <slamd_window/geom/spheres.hpp>

namespace slamd {
namespace _geom {

Spheres::Spheres(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec3>& colors,
    const std::vector<float>& radii,
    float /*min_brightness*/
)
    : positions(positions),
      colors(colors),
      radii(radii) {
    this->mono = Spheres::make_mono_instanced(positions, colors, radii);
}

std::shared_ptr<Spheres> Spheres::deserialize(
    const slamd::flatb::Spheres* spheres_fb
) {
    return std::make_shared<Spheres>(
        gmath::deserialize_vector(spheres_fb->positions()),
        gmath::deserialize_vector(spheres_fb->colors()),
        gmath::deserialize_vector(spheres_fb->radii()),
        spheres_fb->min_brightness()
    );
}

void Spheres::render(
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection
) {
    this->handle_updates();
    this->mono->render(model, view, projection);
}

std::unique_ptr<MonoInstanced> Spheres::make_mono_instanced(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec3>& colors,
    const std::vector<float>& radii
) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;

    slamd::_utils::generate_sphere(vertices, indices, normals, 1.0f, 16, 16);

    return std::make_unique<MonoInstanced>(
        vertices,
        normals,
        indices,
        Spheres::make_transforms(positions, radii),
        colors
    );
}

std::vector<glm::mat4> Spheres::make_transforms(
    const std::vector<glm::vec3>& positions,
    const std::vector<float>& radii
) {
    std::vector<glm::mat4> transforms;
    transforms.reserve(positions.size());

    for (std::size_t i = 0; i < positions.size(); ++i) {
        transforms.push_back(
            slamd::gmath::t3D(positions[i]) * slamd::gmath::scale_all(radii[i])
        );
    }

    return transforms;
}

void Spheres::update_positions(
    const std::vector<glm::vec3>& positions
) {
    this->positions = positions;
    this->pending_trans_update = true;
}

void Spheres::update_colors(
    const std::vector<glm::vec3>& colors
) {
    this->colors = colors;
    this->pending_color_update = true;
}

void Spheres::update_radii(
    const std::vector<float>& radii
) {
    this->radii = radii;
    this->pending_trans_update = true;
}

void Spheres::handle_updates() {
    if (this->pending_trans_update) {
        this->mono->update_transforms(
            Spheres::make_transforms(this->positions, this->radii)
        );
        this->pending_trans_update = false;
    }

    if (this->pending_color_update) {
        this->mono->update_colors(this->colors);
        this->pending_color_update = false;
    }
}

}  // namespace _geom
}  // namespace slamd
