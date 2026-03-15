#pragma once
#include <memory>
#include <optional>
#include <slamd_common/data/image.hpp>
#include <slamd_common/gmath/aabb.hpp>
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/image_texture.hpp>
#include <slamd_window/shaders.hpp>

namespace slamd {
namespace _geom {

class Image : public Geometry {
   private:
    glm::vec2 scale;

   public:
    gl::GLuint vao_id;
    gl::GLuint vbo_id;
    gl::GLuint eab_id;
    std::unique_ptr<graphics::ImageTexture> texture;
    ShaderProgram shader;

   public:
    Image(const slamd::data::Image& image, bool normalized = true);
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    void render(glm::mat4 model, glm::mat4 view, glm::mat4 projection) override;

    static std::shared_ptr<Image> deserialize(
        const slamd::flatb::Image* image_fb
    );

    std::optional<slamd::gmath::AABB> bounds() override;

   private:
    void initialize();
};

}  // namespace _geom
}  // namespace slamd