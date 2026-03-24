#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/utils/mesh.hpp>
#include <slamd_window/geom/box.hpp>

namespace slamd {
namespace _geom {

// clang-format off
// Unit box centered at origin, 6 faces * 4 vertices = 24 unique verts.
// Scaled by dims at construction time.
const std::vector<glm::vec3> box_corners = {{
    // Back face
    {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
    // Front face
    {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f, 0.5f,  0.5f}, {-0.5f, 0.5f,  0.5f},
    // Left face
    {-0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f,  0.5f},
    // Right face
    {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f, 0.5f,  0.5f}, {0.5f, 0.5f, -0.5f},
    // Top face
    {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f,  0.5f}, {-0.5f, 0.5f,  0.5f},
    // Bottom face
    {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f},
}};

const std::vector<uint32_t> box_indices = {{
    0, 2, 1, 0, 3, 2,
    4, 5, 6, 4, 6, 7,
    8, 10, 9, 8, 11, 10,
    12, 13, 14, 12, 14, 15,
    16, 18, 17, 16, 19, 18,
    20, 21, 22, 20, 22, 23
}};

const std::vector<glm::vec3> vertex_normals = {{
    {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
    {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
    {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
    {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
}};
// clang-format on

Box::Box(glm::vec3 dims, glm::vec3 color) {
    std::vector<glm::vec3> scaled(box_corners.size());
    for (size_t i = 0; i < box_corners.size(); i++) {
        scaled[i] = box_corners[i] * dims;
    }

    auto data = slamd::data::MeshDataBuilder()
                     .set_positions(scaled)
                     .set_colors(color)
                     .set_indices(box_indices)
                     .set_normals(vertex_normals)
                     .build();
    this->box_mesh = std::make_unique<Mesh>(std::move(data));
}

void Box::render(
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection
) {
    this->box_mesh->render(model, view, projection);
}

std::shared_ptr<Box> Box::deserialize(
    const slamd::flatb::Box* box_fb
) {
    return std::make_shared<Box>(
        slamd::gmath::deserialize(box_fb->dims()),
        slamd::gmath::deserialize(box_fb->color())
    );
}

}  // namespace _geom
}  // namespace slamd
