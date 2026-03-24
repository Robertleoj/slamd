#include <algorithm>
#include <slamd_window/controls_hint.hpp>

namespace slamd {

void ControlsHint::render() {
    const float margin = 8.0f;

    struct HintLine {
        const char* key;
        const char* action;
    };

    const HintLine lines[] = {
        {"WASD / Right-drag", "Pan"},
        {"Left-drag", "Rotate"},
        {"Scroll", "Zoom"},
        {".", "Re-center"},
        {"G", "Toggle grid"},
    };
    const int n_lines = sizeof(lines) / sizeof(lines[0]);

    const ImGuiStyle& style = ImGui::GetStyle();
    const float col_gap = 16.0f;
    const float pad = 6.0f;

    // Measure column widths.
    float key_w = 0.0f;
    float action_w = 0.0f;
    for (int i = 0; i < n_lines; i++) {
        key_w = std::max(key_w, ImGui::CalcTextSize(lines[i].key).x);
        action_w = std::max(action_w, ImGui::CalcTextSize(lines[i].action).x);
    }

    float line_h = ImGui::CalcTextSize("X").y;
    float total_h = n_lines * line_h + (n_lines - 1) * style.ItemSpacing.y;

    float content_w = key_w + col_gap + action_w;
    float child_w = content_w + pad * 2.0f + 2.0f;
    float child_h = total_h + pad * 2.0f + 2.0f;

    // Position in the bottom-right of the current ImGui window.
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 cr_max = ImGui::GetWindowContentRegionMax();

    ImVec2 overlay_pos(
        win_pos.x + cr_max.x - child_w - margin,
        win_pos.y + cr_max.y - child_h - margin
    );

    ImVec2 saved_cursor = ImGui::GetCursorScreenPos();

    ImGui::PushID("##controls_hint");
    ImGui::SetCursorScreenPos(overlay_pos);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.55f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));

    ImGui::BeginChild(
        "##controls_child",
        ImVec2(child_w, child_h),
        true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs
    );

    for (int i = 0; i < n_lines; i++) {
        ImGui::TextUnformatted(lines[i].key);
        ImGui::SameLine(key_w + col_gap + pad);
        ImGui::TextUnformatted(lines[i].action);
    }

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopID();

    ImGui::SetCursorScreenPos(saved_cursor);
}

}  // namespace slamd
