#include <fmt/format.h>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <slamd_common/data/mesh.hpp>
#include <slamd_common/gmath/serialization.hpp>
#include <slamd_common/utils/mesh.hpp>
#include <slamd_window/constants.hpp>
#include <slamd_window/gen/shader_sources.hpp>
#include <slamd_window/geom/point_cloud.hpp>

namespace slamd {
namespace _geom {

PointCloud::PointCloud(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec3>& colors,
    const std::vector<float>& radii,
    float /*min_brightness*/
)
    : shader(
          shader_source::point_cloud::vert,
          shader_source::point_cloud::frag
      ),
      positions(positions),
      pending_pos_update(false),
      colors(colors),
      pending_colors_update(false),
      radii(radii),
      pending_radii_update(false) {
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
    this->initialize();
}

std::shared_ptr<PointCloud> PointCloud::deserialize(
    const slamd::flatb::PointCloud* point_cloud_fb
) {
    return std::make_shared<PointCloud>(
        gmath::deserialize_vector(point_cloud_fb->positions()),
        gmath::deserialize_vector(point_cloud_fb->colors()),
        gmath::deserialize_vector(point_cloud_fb->radii()),
        point_cloud_fb->min_brightness()
    );
}

std::tuple<size_t, uint, uint> PointCloud::initialize_sphere_mesh() {
    // mesh first
    std::vector<glm::vec3> mesh_vertices;
    std::vector<uint32_t> mesh_indices;
    std::vector<glm::vec3> vertex_normals;

    slamd::_utils::generate_sphere(
        mesh_vertices,
        mesh_indices,
        vertex_normals,
        1.0f,
        3,
        2
    );

    // vertex buffer
    uint mesh_vbo_id;
    gl::glGenBuffers(1, &mesh_vbo_id);
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, mesh_vbo_id);

    size_t vert_size = mesh_vertices.size() * sizeof(glm::vec3);
    size_t normals_size = vertex_normals.size() * sizeof(glm::vec3);

    gl::glBufferData(
        gl::GL_ARRAY_BUFFER,
        vert_size + normals_size,
        nullptr,
        gl::GL_STATIC_DRAW
    );
    gl::glBufferSubData(
        gl::GL_ARRAY_BUFFER,
        0,
        vert_size,
        mesh_vertices.data()
    );
    gl::glBufferSubData(
        gl::GL_ARRAY_BUFFER,
        vert_size,
        normals_size,
        vertex_normals.data()
    );

    // element buffer
    uint mesh_eab_id;
    gl::glGenBuffers(1, &mesh_eab_id);
    gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, mesh_eab_id);

    gl::glBufferData(
        gl::GL_ELEMENT_ARRAY_BUFFER,
        mesh_indices.size() * sizeof(uint32_t),
        mesh_indices.data(),
        gl::GL_STATIC_DRAW
    );

    // and this is the base vertex attribute
    gl::glVertexAttribPointer(
        0,
        3,
        gl::GL_FLOAT,
        gl::GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    gl::glEnableVertexAttribArray(0);

    // then we have the normal attribute
    gl::glVertexAttribPointer(
        1,
        3,
        gl::GL_FLOAT,
        gl::GL_FALSE,
        sizeof(glm::vec3),
        (void*)vert_size
    );
    gl::glEnableVertexAttribArray(1);

    return std::make_tuple(mesh_indices.size(), mesh_vbo_id, mesh_eab_id);
}

uint PointCloud::initialize_pos_buffer() {
    uint pos_vbo_id;
    gl::glGenBuffers(1, &pos_vbo_id);

    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, pos_vbo_id);

    gl::glBufferData(
        gl::GL_ARRAY_BUFFER,
        this->positions.size() * sizeof(glm::vec3),
        this->positions.data(),
        gl::GL_DYNAMIC_DRAW
    );

    gl::glVertexAttribPointer(
        2,
        3,
        gl::GL_FLOAT,
        gl::GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    gl::glEnableVertexAttribArray(2);
    gl::glVertexAttribDivisor(2, 1);

    return pos_vbo_id;
}

uint PointCloud::initialize_radii_buffer() {
    uint vbo_id;
    gl::glGenBuffers(1, &vbo_id);

    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vbo_id);

    gl::glBufferData(
        gl::GL_ARRAY_BUFFER,
        this->radii.size() * sizeof(float),
        this->radii.data(),
        gl::GL_DYNAMIC_DRAW
    );

    gl::glVertexAttribPointer(
        3,
        1,
        gl::GL_FLOAT,
        gl::GL_FALSE,
        sizeof(float),
        (void*)0
    );
    gl::glEnableVertexAttribArray(3);
    gl::glVertexAttribDivisor(3, 1);

    return vbo_id;
}

uint PointCloud::initialize_color_buffer() {
    uint vbo_id;
    gl::glGenBuffers(1, &vbo_id);

    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vbo_id);

    gl::glBufferData(
        gl::GL_ARRAY_BUFFER,
        this->colors.size() * sizeof(glm::vec3),
        this->colors.data(),
        gl::GL_DYNAMIC_DRAW
    );

    gl::glVertexAttribPointer(
        4,
        3,
        gl::GL_FLOAT,
        gl::GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    gl::glEnableVertexAttribArray(4);
    // one per instance
    gl::glVertexAttribDivisor(4, 1);

    return vbo_id;
}

void PointCloud::initialize() {
    gl::glGenVertexArrays(1, &this->vao_id);
    gl::glBindVertexArray(this->vao_id);

    std::tie(this->ball_vertex_count, this->mesh_vbo_id, this->mesh_eab_id) =
        this->initialize_sphere_mesh();

    this->pos_vbo_id = this->initialize_pos_buffer();
    this->radii_vbo_id = this->initialize_radii_buffer();
    this->colors_vbo_id = this->initialize_color_buffer();

    gl::glBindVertexArray(0);
}

void PointCloud::update_positions(
    const std::vector<glm::vec3>& positions
) {
    this->positions = positions;
    this->pending_pos_update = true;
}

void PointCloud::update_colors(
    const std::vector<glm::vec3>& colors
) {
    this->colors = colors;
    this->pending_colors_update = true;
}

void PointCloud::update_radii(
    const std::vector<float>& radii
) {
    this->radii = radii;
    this->pending_radii_update = true;
}

void PointCloud::handle_updates() {
    if (this->pending_pos_update) {
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, this->pos_vbo_id);
        gl::glBufferData(
            gl::GL_ARRAY_BUFFER,
            this->positions.size() * sizeof(glm::vec3),
            this->positions.data(),
            gl::GL_DYNAMIC_DRAW
        );
        this->pending_pos_update = false;
    }

    if (this->pending_colors_update) {
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, this->colors_vbo_id);
        gl::glBufferData(
            gl::GL_ARRAY_BUFFER,
            this->colors.size() * sizeof(glm::vec3),
            this->colors.data(),
            gl::GL_DYNAMIC_DRAW
        );
        this->pending_colors_update = false;
    }

    if (this->pending_radii_update) {
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, this->radii_vbo_id);
        gl::glBufferData(
            gl::GL_ARRAY_BUFFER,
            this->radii.size() * sizeof(float),
            this->radii.data(),
            gl::GL_DYNAMIC_DRAW
        );
        this->pending_radii_update = false;
    }
}

void PointCloud::render(
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection
) {
    gl::glBindVertexArray(this->vao_id);
    this->handle_updates();

    this->shader.use();

    this->shader.set_uniform("u_model", model);
    this->shader.set_uniform("u_view", view);
    this->shader.set_uniform("u_projection", projection);
    gl::glDrawElementsInstanced(
        gl::GL_TRIANGLES,
        this->ball_vertex_count,
        gl::GL_UNSIGNED_INT,
        0,
        this->positions.size()
    );

    gl::glBindVertexArray(0);
}

PointCloud::~PointCloud() {
    gl::glDeleteBuffers(1, &mesh_vbo_id);
    gl::glDeleteBuffers(1, &mesh_eab_id);
    gl::glDeleteBuffers(1, &pos_vbo_id);
    gl::glDeleteBuffers(1, &radii_vbo_id);
    gl::glDeleteBuffers(1, &colors_vbo_id);
    gl::glDeleteVertexArrays(1, &vao_id);
}

}  // namespace _geom
}  // namespace slamd