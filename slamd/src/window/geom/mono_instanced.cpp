#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <spdlog/spdlog.h>
#include <slamd_common/data/mesh.hpp>
#include <slamd_window/constants.hpp>
#include <slamd_window/gen/shader_sources.hpp>
#include <slamd_window/geom/mono_instanced.hpp>

namespace slamd {
namespace _geom {

MonoInstanced::MonoInstanced(
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals,
    const std::vector<uint32_t>& triangle_indices,
    const std::vector<glm::mat4>& transforms,
    const std::vector<glm::vec3>& colors
)
    : vertices(vertices),
      normals(normals),
      triangle_indices(triangle_indices),
      shader(
          shader_source::mono_instanced::vert,
          shader_source::mono_instanced::frag
      ),
      transforms(transforms),
      pending_trans_update(false),
      colors(colors),
      pending_colors_update(false) {
    if (!(vertices.size() == normals.size())) {
        throw std::invalid_argument(fmt::format(
            "number of vertices {}, number of normals {}",
            vertices.size(),
            normals.size()
        ));
    }
    if (!((transforms.size() == colors.size()))) {
        throw std::invalid_argument(fmt::format(
            "number of transforms, and colors got "
            "{} transforms and {} colors",
            transforms.size(),
            colors.size()
        ));
    }
    this->initialize();
}

std::tuple<uint, uint> MonoInstanced::initialize_mesh() {
    // vertex buffer
    uint mesh_vbo_id;
    gl::glGenBuffers(1, &mesh_vbo_id);
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, mesh_vbo_id);

    size_t vert_size = this->vertices.size() * sizeof(glm::vec3);
    size_t normals_size = this->normals.size() * sizeof(glm::vec3);

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
        this->vertices.data()
    );
    gl::glBufferSubData(
        gl::GL_ARRAY_BUFFER,
        vert_size,
        normals_size,
        this->normals.data()
    );

    // element buffer
    uint mesh_eab_id;
    gl::glGenBuffers(1, &mesh_eab_id);
    gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, mesh_eab_id);

    gl::glBufferData(
        gl::GL_ELEMENT_ARRAY_BUFFER,
        this->triangle_indices.size() * sizeof(uint32_t),
        this->triangle_indices.data(),
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

    return std::make_tuple(mesh_vbo_id, mesh_eab_id);
}

uint MonoInstanced::initialize_trans_buffer() {
    uint trans_vbo_id;
    gl::glGenBuffers(1, &trans_vbo_id);

    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, trans_vbo_id);

    gl::glBufferData(
        gl::GL_ARRAY_BUFFER,
        this->transforms.size() * sizeof(glm::mat4),
        this->transforms.data(),
        gl::GL_DYNAMIC_DRAW
    );

    for (int i = 0; i < 4; ++i) {
        gl::glEnableVertexAttribArray(2 + i);
        gl::glVertexAttribPointer(
            2 + i,
            4,
            gl::GL_FLOAT,
            gl::GL_FALSE,
            sizeof(glm::mat4),
            (void*)(sizeof(glm::vec4) * i)
        );
        gl::glVertexAttribDivisor(2 + i, 1);
    }

    return trans_vbo_id;
}

uint MonoInstanced::initialize_color_buffer() {
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
        6,
        3,
        gl::GL_FLOAT,
        gl::GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    gl::glEnableVertexAttribArray(6);
    // one per instance
    gl::glVertexAttribDivisor(6, 1);

    return vbo_id;
}

void MonoInstanced::initialize() {
    gl::glGenVertexArrays(1, &this->vao_id);
    gl::glBindVertexArray(this->vao_id);

    std::tie(this->mesh_vbo_id, this->mesh_eab_id) = this->initialize_mesh();
    this->trans_vbo_id = this->initialize_trans_buffer();
    this->colors_vbo_id = this->initialize_color_buffer();

    gl::glBindVertexArray(0);
}

void MonoInstanced::update_transforms(
    const std::vector<glm::mat4>& transforms
) {
    this->transforms = transforms;
    this->pending_trans_update = true;
}

void MonoInstanced::update_colors(
    const std::vector<glm::vec3>& colors
) {
    this->colors = colors;
    this->pending_colors_update = true;
}

void MonoInstanced::handle_updates() {
    if (this->pending_trans_update) {
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, this->trans_vbo_id);
        gl::glBufferData(
            gl::GL_ARRAY_BUFFER,
            this->transforms.size() * sizeof(glm::mat4),
            this->transforms.data(),
            gl::GL_DYNAMIC_DRAW
        );
        this->pending_trans_update = false;
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
}

void MonoInstanced::render(
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
    this->shader.set_uniform(
        "u_min_brightness",
        _const::default_min_brightness
    );

    gl::glDrawElementsInstanced(
        gl::GL_TRIANGLES,
        this->triangle_indices.size(),
        gl::GL_UNSIGNED_INT,
        0,
        this->transforms.size()
    );

    gl::glBindVertexArray(0);
}

MonoInstanced::~MonoInstanced() {
    gl::glDeleteBuffers(1, &mesh_vbo_id);
    gl::glDeleteBuffers(1, &mesh_eab_id);
    gl::glDeleteBuffers(1, &trans_vbo_id);
    gl::glDeleteBuffers(1, &colors_vbo_id);
    gl::glDeleteVertexArrays(1, &vao_id);
}

}  // namespace _geom
}  // namespace slamd