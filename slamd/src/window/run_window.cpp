/**
 * All of the tree widget stuff is vide-coded!
 */
#include <glbinding/gl/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include <slamd_window/frame_timer.hpp>
#include <slamd_window/glfw.hpp>
#include <slamd_window/run_window.hpp>
#include <thread>

namespace slamd {

void framebuffer_size_callback(
    GLFWwindow* window,
    int width,
    int height
) {
    (void)window;
    gl::glViewport(0, 0, width, height);
}

// Vibe coded
inline float compute_visible_tree_content_width(
    Node* root,
    const std::unordered_map<Node*, bool>& open_map
) {
    const ImGuiStyle& style = ImGui::GetStyle();
    const float indent = style.IndentSpacing;
    const float row_h = ImGui::GetFrameHeight();
    const float chk_pad = style.ItemInnerSpacing.x;
    const float label_pad = ImGui::GetTreeNodeToLabelSpacing();
    const float badge_w = 12.0f;

    float max_w = 0.0f;

    std::function<void(Node*, int)> walk = [&](Node* n, int depth) {
        float left = depth * indent + row_h + chk_pad +
                     (n->has_children_cached ? row_h : 0.0f) + label_pad;

        float total = left + n->last_label_w + badge_w;
        if (total > max_w) {
            max_w = total;
        }

        auto it = open_map.find(n);
        bool is_open = (it != open_map.end()) ? it->second : false;
        if (n->has_children_cached && is_open) {
            for (auto& kv : n->children) {
                walk(kv.second.get(), depth + 1);
            }
        }
    };

    walk(root, 0);
    return max_w;
}

// Vibe coded
inline void tree_menu(
    View* view
) {
    Node* root = view->tree->root.get();

    const float min_field_w = 250.0f;
    const float text_w = ImGui::CalcTextSize(view->filter_buf).x;
    const float pad_x = ImGui::GetStyle().FramePadding.x;

    const float desired_field_w = text_w + 2.0f * pad_x + 12.0f;
    const float field_w =
        (desired_field_w < min_field_w) ? min_field_w : desired_field_w;

    std::optional<std::string> filter_error_text;
    std::optional<TreePath> filter_path;
    std::string filter_string(view->filter_buf);

    if (filter_string.size() > 0) {
        try {
            filter_path = TreePath(filter_string);
        } catch (const std::invalid_argument& e) {
            filter_error_text = e.what();
        }
    }

    view->tree->mark_nodes_matching_glob(filter_path);

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Filter:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(field_w);

    if (filter_error_text.has_value()) {
        ImGui::PushStyleColor(
            ImGuiCol_FrameBg,
            ImVec4(0.35f, 0.10f, 0.10f, 1.0f)
        );
        ImGui::PushStyleColor(
            ImGuiCol_FrameBgHovered,
            ImVec4(0.45f, 0.12f, 0.12f, 1.0f)
        );
        ImGui::PushStyleColor(
            ImGuiCol_FrameBgActive,
            ImVec4(0.50f, 0.13f, 0.13f, 1.0f)
        );
        ImGui::PushStyleColor(
            ImGuiCol_Border,
            ImVec4(0.90f, 0.20f, 0.20f, 1.0f)
        );
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    }
    if (ImGui::InputText(
            "##filter",
            view->filter_buf,
            IM_ARRAYSIZE(view->filter_buf)
        )) {
        view->mark_dirty();
    }
    if (filter_error_text.has_value()) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", filter_error_text->c_str());
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImVec2 rmin = ImGui::GetItemRectMin();
        ImVec2 rmax = ImGui::GetItemRectMax();
        const ImGuiStyle& style = ImGui::GetStyle();

        const char* bang = "!";
        ImVec2 bang_sz = ImGui::CalcTextSize(bang);

        float x = rmax.x - style.FramePadding.x - bang_sz.x;
        float y = rmin.y + ((rmax.y - rmin.y) - bang_sz.y) * 0.5f;

        dl->AddText(
            ImVec2(x, y),
            ImGui::GetColorU32(ImVec4(0.95f, 0.25f, 0.25f, 1.0f)),
            bang
        );
    }

    ImGui::Separator();

    std::function<void(Node*, std::string, int, bool, TreePath&)> draw_node =
        [&](Node* n,
            std::string label,
            int depth,
            bool parent_dimmed,
            TreePath& full_path) {
            ImGui::PushID(n);

            if (filter_path.has_value()) {
                n->glob_matches = full_path.matches_glob(filter_path.value());
            } else {
                n->glob_matches = std::nullopt;
            }

            const bool has_children = !n->children.empty();
            const ImGuiTreeNodeFlags base_flags =
                ImGuiTreeNodeFlags_SpanAvailWidth |
                ImGuiTreeNodeFlags_SpanFullWidth |
                ImGuiTreeNodeFlags_FramePadding |
                (has_children ? 0
                              : ImGuiTreeNodeFlags_Leaf |
                                    ImGuiTreeNodeFlags_NoTreePushOnOpen);

            const bool dimmed = parent_dimmed || !n->checked;
            bool dimmed_here = !parent_dimmed && !n->checked;
            if (dimmed_here) {
                float base_alpha = ImGui::GetStyle().Alpha;
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, base_alpha * 0.45f);
            }

            if (ImGui::Checkbox("##visible", &n->checked)) {
                view->mark_dirty();
            }
            ImGui::SameLine(0.0f, 4.0f);

            bool open = false;
            n->last_label_w = ImGui::CalcTextSize(label.c_str()).x;
            n->has_children_cached = has_children;

            if (has_children) {
                open = ImGui::TreeNodeEx(label.c_str(), base_flags);
                view->tree_open[n] = open;
            } else {
                view->tree_open[n] = false;
                ImGui::TreeNodeEx(label.c_str(), base_flags);
            }

            if (n->glob_matches.has_value()) {
                const bool match = n->glob_matches.value_or(false);

                ImVec2 item_min = ImGui::GetItemRectMin();
                ImVec2 item_max = ImGui::GetItemRectMax();

                ImVec2 win_pos = ImGui::GetWindowPos();
                ImVec2 cr_max = ImGui::GetWindowContentRegionMax();
                float right_edge_x =
                    win_pos.x + cr_max.x - ImGui::GetStyle().ItemInnerSpacing.x;

                float radius = 4.0f;
                float cx = right_edge_x - radius;
                float cy = 0.5f * (item_min.y + item_max.y);

                ImU32 col_fill =
                    ImGui::GetColorU32(ImVec4(0.30f, 0.85f, 0.40f, 1.0f));
                ImU32 col_line = ImGui::GetColorU32(
                    ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)
                );

                ImDrawList* dl = ImGui::GetWindowDrawList();
                if (match) {
                    dl->AddCircleFilled(ImVec2(cx, cy), radius, col_fill);
                } else {
                    dl->AddCircle(ImVec2(cx, cy), radius, col_line, 12, 1.5f);
                }
            }

            if (open) {
                for (const auto& c : n->children) {
                    full_path.components.push_back(c.first);
                    draw_node(
                        c.second.get(),
                        c.first,
                        depth + 1,
                        dimmed,
                        full_path
                    );
                    full_path.components.pop_back();
                }
                ImGui::TreePop();
            }
            if (dimmed_here) {
                ImGui::PopStyleVar();
            }
            ImGui::PopID();
        };

    TreePath pth("/");
    draw_node(root, "/", 0, false, pth);
}

// Vibe coded
inline void draw_tree_overlay(
    View* view,
    const char* child_id = "##scene_tree_overlay_child",
    const char* header = "Tree",
    float margin = 8.0f,
    float min_width = 220.0f,
    float min_height = 80.0f
) {
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 cr_min = ImGui::GetWindowContentRegionMin();
    ImVec2 cr_max = ImGui::GetWindowContentRegionMax();
    ImVec2 tl(win_pos.x + cr_min.x, win_pos.y + cr_min.y);
    ImVec2 br(win_pos.x + cr_max.x, win_pos.y + cr_max.y);

    ImVec2 pos = {tl.x + margin, tl.y + margin};
    float max_w = std::max(0.0f, (br.x - tl.x) - 2.0f * margin);
    float max_h = std::max(0.0f, (br.y - tl.y) - 2.0f * margin);

    const ImGuiStyle& style = ImGui::GetStyle();
    const float pad_x = style.WindowPadding.x;
    const float border_px = 1.0f;

    const char* filter_lbl = "Filter:";
    float filter_lbl_w = ImGui::CalcTextSize(filter_lbl).x;
    float text_w = ImGui::CalcTextSize(view->filter_buf).x;
    float field_w =
        std::max(250.0f, text_w + 2.0f * style.FramePadding.x + 12.0f);
    float filter_row_w = filter_lbl_w + style.ItemSpacing.x + field_w;

    float tree_w = compute_visible_tree_content_width(
        view->tree->root.get(),
        view->tree_open
    );

    float header_w = 0.0f;
    if (header && *header) {
        header_w = ImGui::CalcTextSize(header).x;
    }

    float need_content_w = std::max({filter_row_w, tree_w, header_w});

    float predicted_child_w = need_content_w + pad_x * 2.0f + border_px * 2.0f;

    float prev_w =
        (view->tree_overlay_w > 0.0f) ? view->tree_overlay_w : min_width;
    float target_w = std::clamp(predicted_child_w, min_width, max_w);

    auto snap = [](float v) {
        return std::floor(v + 0.5f);
    };
    const float hysteresis_px = 2.0f;
    if (std::fabs(target_w - prev_w) <= hysteresis_px) {
        target_w = prev_w;
    }
    target_w = snap(target_w);

    float prev_h =
        (view->tree_overlay_h > 0.0f) ? view->tree_overlay_h : min_height;
    float target_h = std::clamp(prev_h, min_height, max_h);

    ImVec2 prev_cursor = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(pos);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.55f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));

    ImGuiWindowFlags child_flags =
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;

    ImGui::BeginChild(child_id, ImVec2(target_w, target_h), true, child_flags);

    ImGui::BeginGroup();
    if (header && *header) {
        ImGui::TextUnformatted(header);
        ImGui::Separator();
    }
    tree_menu(view);
    ImGui::EndGroup();

    ImVec2 content_min = ImGui::GetItemRectMin();
    ImVec2 content_max = ImGui::GetItemRectMax();
    ImVec2 content_sz =
        ImVec2(content_max.x - content_min.x, content_max.y - content_min.y);

    ImGui::EndChild();

    float measured_child_w =
        content_sz.x + style.WindowPadding.x * 2.0f + border_px * 2.0f;
    float measured_child_h =
        content_sz.y + style.WindowPadding.y * 2.0f + border_px * 2.0f;

    auto apply_axis = [&](float want, float prev, float mn, float mx) {
        want = std::clamp(want, mn, mx);
        if (std::fabs(want - prev) <= hysteresis_px) {
            want = prev;
        }
        return snap(want);
    };

    view->tree_overlay_w = apply_axis(
        std::max(measured_child_w, predicted_child_w),
        target_w,
        min_width,
        max_w
    );
    view->tree_overlay_h =
        apply_axis(measured_child_h, target_h, min_height, max_h);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::SetCursorScreenPos(prev_cursor);
}

void run_window(
    StateManager& state_manager
) {
    auto window = glutils::make_window("Slam Dunk", 1000, 1000);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    FrameTimer frame_timer;
    bool loaded_layout = false;
    bool checked_layout = false;

    while (!glfwWindowShouldClose(window)) {
        bool had_updates = state_manager.apply_updates();
        if (had_updates) {
            for (auto& [name, view] : state_manager.views) {
                view->mark_dirty();
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!state_manager.loaded) {
            // render a window with a message
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Always);
            ImGui::Begin("Waiting for Connection", nullptr);
            ImGui::TextWrapped(
                "Hold tight, bro! We're just waiting for that connection to "
                "come through..."
            );
            ImGui::End();

        } else {
            ImGuiID main_dockspace_id;

            if (!loaded_layout && !checked_layout) {
                if (state_manager.layout_path.has_value()) {
                    auto layout_path = state_manager.layout_path.value();
                    if (fs::exists(layout_path)) {
                        ImGui::LoadIniSettingsFromDisk(
                            layout_path.string().c_str()
                        );
                        loaded_layout = true;
                    }
                    checked_layout = true;
                }
            }

            main_dockspace_id = ImGui::DockSpaceOverViewport();

            for (auto& [scene_name, scene] : state_manager.views) {
                if (!loaded_layout) {
                    ImGui::SetNextWindowDockID(
                        main_dockspace_id,
                        ImGuiCond_Once
                    );
                }
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

                ImGui::Begin(
                    scene_name.c_str(),
                    nullptr,
                    ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse
                );
                scene->render_to_imgui();

                draw_tree_overlay(scene.get());

                ImGui::End();
                ImGui::PopStyleVar();
            }
        }

        ImGui::Render();

        gl::glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        gl::glClear(gl::GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_timer.log_frame();
        float frame_time = frame_timer.timedelta();
        constexpr double target_frame_time = 1.0 / 120.0;

        if (frame_time < target_frame_time) {
            float sleep_duration = target_frame_time - frame_time;
            std::this_thread::sleep_for(
                std::chrono::duration<float>(sleep_duration)
            );
        }
    }

    SPDLOG_INFO("Window closed!");

    if (state_manager.layout_path.has_value()) {
        auto layout_path = state_manager.layout_path.value();
        SPDLOG_INFO("Saving layout to {}", layout_path.string());
        ImGui::SaveIniSettingsToDisk(layout_path.string().c_str());
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}
}  // namespace slamd