#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>
#include <slamd_window/gen/shader_sources.hpp>
#include <slamd_window/geom/arcball_indicator.hpp>
#include <slamd_window/paths.hpp>

namespace slamd {
namespace _geom {

void ArcballIndicator::initialize() {
    // clang-format off
    std::vector<float> verts = {
        // x cross
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        // y cross
        0.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        // z bar
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    // clang-format on

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

ArcballIndicator::ArcballIndicator()
    : shader(
          shader_source::arcball_indicator::vert,
          shader_source::arcball_indicator::frag
      ) {
    this->initialize();
}

float ArcballIndicator::get_alpha() {
    // smoothly decrease alpha until 0
    // should take ~1sec to completely fade
    if (!this->last_interacted.has_value()) {
        return 0.0f;
    }

    auto now = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> sec_duration = now - last_interacted.value();
    float secs = sec_duration.count();

    float start_fading = 0.2f;
    float fade_speed = 0.2f;

    return glm::clamp(1.0f - ((secs - start_fading) / fade_speed), 0.0f, 1.0f);
}

void ArcballIndicator::interact() {
    this->last_interacted = std::chrono::high_resolution_clock::now();
}

glm::mat4 ArcballIndicator::get_scale_mat(
    float scale
) {
    return glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
}

void ArcballIndicator::render(
    glm::mat4 model,
    glm::mat4 view,
    glm::mat4 projection
) {
    auto alpha = this->get_alpha();
    if (alpha < 1e-6) {
        // no need to draw
        return;
    }

    auto scale_mat =
        ArcballIndicator::get_scale_mat(this->arcball_zoom / 20.0f);
    this->shader.use();
    this->shader.set_uniform("uModel", model * scale_mat);
    this->shader.set_uniform("uView", view);
    this->shader.set_uniform("uProjection", projection);
    this->shader.set_uniform(
        "uColor",
        glm::vec3(1.0f, 1.0f, 1.0f)
    );  // clean gray
    this->shader.set_uniform("uAlpha", alpha);

    gl::glBindVertexArray(this->vao_id);
    gl::glDrawArrays(gl::GL_LINES, 0, this->vertex_count);

    gl::glBindVertexArray(0);
};

bool ArcballIndicator::is_animating() const {
    if (!this->last_interacted.has_value()) {
        return false;
    }
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> secs = now - last_interacted.value();
    return secs.count() < 0.4f;
}

void ArcballIndicator::set_arcball_zoom(
    float zoom
) {
    this->arcball_zoom = zoom;
}

ArcballIndicator::~ArcballIndicator() {
    if (this->vbo_id) {
        gl::glDeleteBuffers(1, &this->vbo_id);
        this->vbo_id = 0;
    }
    if (this->vao_id) {
        gl::glDeleteVertexArrays(1, &this->vao_id);
        this->vao_id = 0;
    }
}

}  // namespace _geom
}  // namespace slamd