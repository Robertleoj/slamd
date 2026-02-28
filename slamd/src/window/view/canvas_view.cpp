
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <slamd_common/gmath/rect2d.hpp>
#include <slamd_window/view/canvas_view.hpp>

namespace slamd {

CanvasView::CanvasView(
    std::shared_ptr<Tree> tree
)
    : View(std::move(tree)),
      frame_buffer(500, 500),
      camera(slamd::gmath::Rect2D({0.0, 0.0}, {1.0, 1.0})),
      manually_moved(false) {}

void CanvasView::render_to_imgui() {
    ImVec2 full_size = ImGui::GetContentRegionAvail();
    float overlay_height = 24.0f;
    ImVec2 image_size = ImVec2(full_size.x, full_size.y - overlay_height);

    int width = static_cast<int>(image_size.x);
    int height = static_cast<int>(image_size.y);
    bool resized = this->frame_buffer.rescale(width, height);

    if (!this->manually_moved) {
        this->set_default_pos();
    } else {
        this->fix_view_aspect();
    }

    bool need_render = this->_dirty || resized;

    if (need_render) {
        this->render_to_frame_buffer();
        this->_dirty = false;
    }

    ImGui::Image(
        (ImTextureID)this->frame_buffer.frame_texture(),
        image_size,
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    this->handle_input();

    glm::vec2 world_mouse_pos =
        this->camera.get_world_coords(this->get_normalized_mouse_pos());

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.85f));
    if (ImGui::BeginChild(
            "MouseInfoStrip",
            ImVec2(full_size.x, overlay_height),
            false
        )) {
        ImVec2 text_size = ImGui::CalcTextSize("Mouse: (0000.0, 0000.0)");
        ImVec2 win_size = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(
            (win_size.x - text_size.x) * 0.5f,
            (win_size.y - text_size.y) * 0.5f
        ));
        ImGui::Text(
            "Mouse: (%.1f, %.1f)",
            world_mouse_pos.x,
            world_mouse_pos.y
        );
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    this->frame_timer.log_frame();
}

void CanvasView::render_to_frame_buffer() {
    this->frame_buffer.bind();

    gl::glEnable(gl::GL_BLEND);
    gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);

    gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
    gl::glEnable(gl::GL_DEPTH_TEST);

    gl::glClearColor(0.5686f, 0.0980f, 0.4196f, 1.0f);
    gl::glClear(gl::GL_COLOR_BUFFER_BIT);

    glm::mat4 projection_matrix = this->camera.get_projection_matrix();

    // TODO: render calls here
    this->tree->render(glm::mat4(1.0), projection_matrix);

    this->frame_buffer.unbind();
    this->frame_buffer.resolve();
}

void CanvasView::handle_input() {
    // TODO: handle input
    if (ImGui::IsWindowFocused()) {
        this->handle_translation_input();
        this->handle_mouse_input();

        if (ImGui::IsKeyPressed(ImGuiKey_Period, false)) {
            this->set_default_pos();
            this->manually_moved = false;
            this->mark_dirty();
        }
    }
}

void CanvasView::handle_mouse_input() {
    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            // handle dragging
            auto mouse_drag_delta =
                ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

            float mouse_drag_delta_x =
                static_cast<float>(mouse_drag_delta.x) /
                static_cast<float>(this->frame_buffer.width());

            float mouse_drag_delta_y =
                static_cast<float>(mouse_drag_delta.y) /
                static_cast<float>(this->frame_buffer.height());
            ;

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

            this->camera.translate_normalized(
                {-mouse_drag_delta_x, -mouse_drag_delta_y}
            );
            this->manually_moved = true;
            this->mark_dirty();
        }

        if (io.MouseWheel != 0.0f) {
            auto scroll_input = static_cast<float>(io.MouseWheel);

            float zoom_amount = static_cast<float>(scroll_input) * 0.1f;

            this->camera.zoom_relative(
                zoom_amount,
                this->get_normalized_mouse_pos()
            );

            this->manually_moved = true;
            this->mark_dirty();
        }
    }
}

glm::vec2 CanvasView::get_normalized_mouse_pos() {
    ImVec2 mouse_pos_global = ImGui::GetMousePos();
    ImVec2 viewport_loc = ImGui::GetItemRectMin();

    float mouse_pos_x =
        static_cast<float>(mouse_pos_global.x - viewport_loc.x) /
        static_cast<float>(this->frame_buffer.width());
    float mouse_pos_y =
        (static_cast<float>(mouse_pos_global.y - viewport_loc.y) /
         static_cast<float>(this->frame_buffer.height()));

    return glm::vec2(mouse_pos_x, mouse_pos_y);
}

void CanvasView::handle_translation_input() {
    glm::vec2 translation(0.0f, 0.0f);
    float movement_amount = this->frame_timer.timedelta();

    float zoom = 0.0f;

    glm::vec2 right(movement_amount, 0.0f);
    glm::vec2 up(0.0, -movement_amount);

    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        translation += right;
    }

    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        translation -= right;
    }

    if (ImGui::IsKeyDown(ImGuiKey_E)) {
        translation += up;
    }

    if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        translation -= up;
    }

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        zoom += movement_amount;
    }

    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        zoom -= movement_amount;
    }

    if (glm::length(translation) > 1e-6f) {
        this->manually_moved = true;
        this->camera.translate_relative(translation);
        this->mark_dirty();
    }

    if (glm::abs(zoom) > 1e-6f) {
        this->manually_moved = true;
        this->camera.zoom_relative(zoom);
        this->mark_dirty();
    }
}

void CanvasView::set_default_pos() {
    auto maybe_bounds = this->tree->bounds();

    slamd::gmath::AABB bounds = maybe_bounds.has_value()
                                    ? maybe_bounds.value()
                                    : slamd::gmath::AABB(
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(1.0f, 1.0f, 0.0f)
                                      );

    float window_aspect = this->frame_buffer.aspect();

    this->camera.set_viewport(slamd::gmath::Rect2D::center_cover(
        slamd::gmath::Rect2D::from_aabb3d(bounds),
        window_aspect
    ));
}

void CanvasView::fix_view_aspect() {
    float window_aspect = this->frame_buffer.aspect();
    this->camera.set_viewport(
        slamd::gmath::Rect2D::center_cover(this->camera.viewport, window_aspect)
    );
}

}  // namespace slamd