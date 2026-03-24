#include <glm/gtc/matrix_transform.hpp>
#include <slamd_common/data/mesh.hpp>
#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/numbers.hpp>
#include <slamd_window/geom/poly_line.hpp>

namespace slamd {
namespace _geom {

std::shared_ptr<PolyLine> PolyLine::deserialize(
    const slamd::flatb::PolyLine* poly_line_fb
) {
    return std::make_shared<PolyLine>(
        slamd::gmath::deserialize_vector(poly_line_fb->points()),
        poly_line_fb->thickness(),
        slamd::gmath::deserialize(poly_line_fb->color()),
        poly_line_fb->min_brightness()
    );
}

const float BEND_THRESHOLD = 0.85f;  // cos(~30°)

// Insert arc-interpolated points at sharp bends so the tube rounds corners
// instead of pinching or ballooning.
std::vector<glm::vec3> subdivide_sharp_bends(
    const std::vector<glm::vec3>& points,
    float radius
) {
    size_t n = points.size();
    if (n < 3) {
        return points;
    }

    // First pass: determine which interior points are sharp bends.
    std::vector<bool> is_sharp(n, false);
    for (size_t i = 1; i < n - 1; i++) {
        glm::vec3 incoming = glm::normalize(points[i] - points[i - 1]);
        glm::vec3 outgoing = glm::normalize(points[i + 1] - points[i]);
        is_sharp[i] = glm::dot(incoming, outgoing) < BEND_THRESHOLD;
    }

    // Second pass: compute per-corner pullback so adjacent bends don't
    // overlap. Each segment is shared by at most two corners (its start
    // and end). Each corner gets at most half of each adjacent segment.
    // If both ends of a segment are sharp, each gets a quarter.
    std::vector<float> pullback(n, 0.0f);
    for (size_t i = 1; i < n - 1; i++) {
        if (!is_sharp[i]) continue;

        float seg_before = glm::length(points[i] - points[i - 1]);
        float seg_after = glm::length(points[i + 1] - points[i]);

        float budget_before =
            seg_before * (is_sharp[i - 1] ? 0.25f : 0.5f);
        float budget_after =
            seg_after * ((i + 1 < n - 1 && is_sharp[i + 1]) ? 0.25f : 0.5f);

        pullback[i] =
            glm::min(radius * 2.0f, glm::min(budget_before, budget_after));
    }

    // Third pass: emit points with Bezier arcs at sharp bends.
    std::vector<glm::vec3> result;
    result.push_back(points[0]);

    for (size_t i = 1; i < n - 1; i++) {
        if (!is_sharp[i]) {
            result.push_back(points[i]);
            continue;
        }

        glm::vec3 incoming = glm::normalize(points[i] - points[i - 1]);
        glm::vec3 outgoing = glm::normalize(points[i + 1] - points[i]);
        float cos_angle = glm::dot(incoming, outgoing);

        glm::vec3 p_in = points[i] - incoming * pullback[i];
        glm::vec3 p_out = points[i] + outgoing * pullback[i];

        int steps = glm::clamp(
            static_cast<int>((1.0f - cos_angle) * 5.0f), 2, 8
        );

        for (int s = 0; s <= steps; s++) {
            float t = static_cast<float>(s) / static_cast<float>(steps);
            glm::vec3 a = glm::mix(p_in, points[i], t);
            glm::vec3 b = glm::mix(points[i], p_out, t);
            result.push_back(glm::mix(a, b, t));
        }
    }

    result.push_back(points.back());
    return result;
}

std::unique_ptr<Mesh> make_poly_line_mesh(
    const std::vector<glm::vec3>& input_points,
    float thickness,
    const glm::vec3& color,
    float min_brightness
) {
    const auto points =
        subdivide_sharp_bends(input_points, thickness * 0.5f);

    std::vector<glm::vec3> verts;
    std::vector<uint32_t> indices;

    if (points.size() < 2) {
        throw std::invalid_argument("not enough points");
    }

    const int segments = 8;
    const float radius = thickness * 0.5f;

    glm::vec3 prev_forward = glm::normalize(points[1] - points[0]);
    glm::vec3 prev_up = glm::vec3(0, 1, 0);
    if (glm::abs(glm::dot(prev_forward, prev_up)) > 0.9f) {
        prev_up = glm::vec3(1, 0, 0);
    }

    for (size_t i = 0; i < points.size(); i++) {
        glm::vec3 forward;
        if (i == 0) {
            forward = glm::normalize(points[1] - points[0]);
        } else if (i == points.size() - 1) {
            forward = glm::normalize(points[i] - points[i - 1]);
        } else {
            forward = glm::normalize(points[i + 1] - points[i]);
        }

        // Smooth frame transport
        glm::vec3 axis = glm::cross(prev_forward, forward);
        float axis_len = glm::length(axis);
        if (axis_len > 1e-6f) {
            float angle = glm::asin(axis_len);
            axis = glm::normalize(axis);
            glm::mat3 rot =
                glm::mat3(glm::rotate(glm::mat4(1.0f), angle, axis));
            prev_up = rot * prev_up;
        }

        glm::vec3 right = glm::normalize(glm::cross(forward, prev_up));
        glm::vec3 local_up = glm::normalize(glm::cross(right, forward));

        prev_forward = forward;

        uint32_t base = static_cast<uint32_t>(verts.size());

        for (int j = 0; j < segments; ++j) {
            float angle = (j / (float)segments) * 2 * _num::pi;
            glm::vec3 offset =
                glm::cos(angle) * right + glm::sin(angle) * local_up;
            verts.push_back(points[i] + offset * radius);
        }

        if (i > 0) {
            for (int j = 0; j < segments; ++j) {
                int curr = j;
                int next = (j + 1) % segments;

                uint32_t a = base - segments + curr;
                uint32_t b = base - segments + next;
                uint32_t c = base + next;
                uint32_t d = base + curr;

                indices.push_back(a);
                indices.push_back(c);
                indices.push_back(b);

                indices.push_back(a);
                indices.push_back(d);
                indices.push_back(c);
            }
        }
    }

    auto data = slamd::data::MeshDataBuilder()
                    .set_positions(verts)
                    .set_colors(color)
                    .set_indices(indices)
                    .compute_normals()
                    .build();

    return std::make_unique<Mesh>(data, min_brightness);
}

PolyLine::PolyLine(
    const std::vector<glm::vec3>& points,
    float thickness,
    const glm::vec3& color,
    float min_brightness
)
    : mesh(make_poly_line_mesh(points, thickness, color, min_brightness)) {}

void PolyLine::render(
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection
) {
    mesh->render(model, view, projection);
}

}  // namespace _geom
}  // namespace slamd