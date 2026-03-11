#include <glbinding/gl/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include <slamd_window/glfw.hpp>
#include <slamd_window/run_window.hpp>

namespace slamd {

void framebuffer_size_callback(
    GLFWwindow* window,
    int width,
    int height
) {
    (void)window;
    gl::glViewport(0, 0, width, height);
}

void run_window(
    StateManager& state_manager
) {
    auto window = glutils::make_window("Slam Dunk", 1000, 1000);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);  // Enable VSync for smooth frame pacing

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
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

                if (scene->tree_overlay.render(scene->tree)) {
                    scene->mark_dirty();
                }

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
