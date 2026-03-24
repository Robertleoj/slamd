#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>
#include <slamd_window/gen/shader_sources.hpp>
#include <slamd_window/geom/xy_grid.hpp>
#include <slamd_window/paths.hpp>

namespace slamd {
namespace _geom {

glm::mat4 get_scale_mat(
    float log_scale
) {
    float scale = pow(10.0f, log_scale);
    return glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 0.0));
}

void GridXYPlane::initialize() {
    std::vector<float> verts;

    for (float x = -this->grid_size; x <= this->grid_size; x++) {
        verts.insert(
            verts.end(),
            {x, -this->grid_size, 0.0f, x, this->grid_size, 0.0f}
        );
    }

    for (float y = -this->grid_size; y <= this->grid_size; y++) {
        verts.insert(
            verts.end(),
            {-this->grid_size, y, 0.0f, this->grid_size, y, 0.0f}
        );
    }

    this->vertex_count = verts.size() / 3;

    gl::glGenVertexArrays(1, &this->vao_id);
    gl::glGenBuffers(1, &this->vbo_id);

    gl::glBindVertexArray(this->vao_id);
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, this->vbo_id);
    gl::glBufferData(
        gl::GL_ARRAY_BUFFER,
        verts.size() * sizeof(float),
        verts.data(),
        gl::GL_STATIC_DRAW
    );

    gl::glEnableVertexAttribArray(0);
    gl::glVertexAttribPointer(
        0,
        3,
        gl::GL_FLOAT,
        gl::GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    gl::glBindVertexArray(0);
}

GridXYPlane::GridXYPlane(
    float grid_size
)
    : shader(shader_source::xy_grid::vert, shader_source::xy_grid::frag),
      grid_size(grid_size) {
    this->initialize();
}

float calculate_alpha(
    float log_best,
    float level
) {
    return glm::clamp(1.0f - std::fabs(log_best - level), 0.0f, 1.0f);
}

void GridXYPlane::render(
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection
) {
    float desired_scale = this->arcball_zoom / 10.0;

    float log_desired_scale = log10(desired_scale);

    // render three grids - at least one with the closest scale
    float best_log_scale = round(log_desired_scale);
    float log_scale_below = best_log_scale - 1;
    float log_scale_above = best_log_scale + 1;

    float best_alpha = calculate_alpha(log_desired_scale, best_log_scale);
    float below_alpha = calculate_alpha(log_desired_scale, log_scale_below);
    float above_alpha = calculate_alpha(log_desired_scale, log_scale_above);

    glm::mat4 best_scale_mat = get_scale_mat(best_log_scale);
    glm::mat4 below_scale_mat = get_scale_mat(log_scale_below);
    glm::mat4 above_scale_mat = get_scale_mat(log_scale_above);

    // Assuming your shader is already bound
    // And has these uniforms: uModel, uView, uProjection, uColor
    this->shader.use();
    this->shader.set_uniform("uView", view);
    this->shader.set_uniform("uProjection", projection);
    this->shader.set_uniform(
        "uColor",
        glm::vec3(0.6f, 0.6f, 0.6f)
    );  // clean gray

    // render best scale
    this->shader.set_uniform("uModel", model * best_scale_mat);
    this->shader.set_uniform("uExtraAlpha", best_alpha);
    this->shader.set_uniform("uScale", desired_scale);

    gl::glBindVertexArray(this->vao_id);
    gl::glDepthMask(gl::GL_FALSE);  // don't mess with z-buffer

    gl::glDrawArrays(gl::GL_LINES, 0, this->vertex_count);

    float draw_threshold = 0.01;

    if (below_alpha > draw_threshold) {
        this->shader.set_uniform("uModel", model * below_scale_mat);
        this->shader.set_uniform("uExtraAlpha", below_alpha);

        gl::glDrawArrays(gl::GL_LINES, 0, this->vertex_count);
    }
    if (above_alpha > draw_threshold) {
        this->shader.set_uniform("uModel", model * above_scale_mat);
        this->shader.set_uniform("uExtraAlpha", above_alpha);

        gl::glDrawArrays(gl::GL_LINES, 0, this->vertex_count);
    }
    gl::glDepthMask(gl::GL_TRUE);

    gl::glBindVertexArray(0);
}

GridXYPlane::~GridXYPlane() {
    gl::glDeleteBuffers(1, &this->vbo_id);
    gl::glDeleteVertexArrays(1, &this->vao_id);
}


void GridXYPlane::set_arcball_zoom(
    float zoom
) {
    this->arcball_zoom = zoom;
}

}  // namespace _geom

}  // namespace slamd