#include <functional>
#include <imgui.h>
#include <slamd_window/tree_overlay.hpp>

namespace slamd {

TreeOverlay::TreeOverlay() {}

bool TreeOverlay::render(
    std::shared_ptr<Tree> tree
) {
    dirty = false;
    current_tree = tree;
    Node* root = tree->root.get();

    // Parse the filter
    std::string filter_string(filter_buf);
    filter_path = std::nullopt;
    filter_error = std::nullopt;

    if (!filter_string.empty()) {
        try {
            filter_path = TreePath(filter_string);
        } catch (const std::invalid_argument& e) {
            filter_error = e.what();
        }
    }
    tree->mark_nodes_matching_glob(filter_path);

    // Position in the upper-left of the current ImGui window's content area.
    const float margin = 8.0f;
    const float min_w = 220.0f;
    const float min_h = 80.0f;

    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 cr_min = ImGui::GetWindowContentRegionMin();
    ImVec2 cr_max = ImGui::GetWindowContentRegionMax();

    float avail_w = cr_max.x - cr_min.x - 2.0f * margin;
    float avail_h = cr_max.y - cr_min.y - 2.0f * margin;

    // Compute width from actual content (filter row + tree nodes).
    const ImGuiStyle& style = ImGui::GetStyle();
    float filter_row_w = compute_filter_row_width();
    float tree_w = compute_tree_content_width(root);
    float header_w = ImGui::CalcTextSize("Tree").x;
    float content_w = std::max({filter_row_w, tree_w, header_w});
    float child_w = content_w + style.WindowPadding.x * 2.0f + 2.0f;

    // Compute height from actual content (header + filter + tree rows).
    float row_h = ImGui::GetFrameHeightWithSpacing();
    float separator_h = style.ItemSpacing.y + 1.0f;
    // Header row + separator + filter row + separator + tree rows
    float tree_h = compute_tree_content_height(root);
    float content_h = row_h + separator_h + row_h + separator_h + tree_h;
    float child_h = content_h + style.WindowPadding.y * 2.0f + 2.0f;

    float w = std::clamp(child_w, min_w, avail_w);
    float h = std::clamp(child_h, min_h, avail_h);

    ImVec2 overlay_pos(
        win_pos.x + cr_min.x + margin,
        win_pos.y + cr_min.y + margin
    );

    // Save and restore the parent window's cursor position so the overlay
    // doesn't affect layout.
    ImVec2 saved_cursor = ImGui::GetCursorScreenPos();

    // Use a unique ID per parent window to keep overlays independent.
    ImGui::PushID("##tree_overlay");

    ImGui::SetCursorScreenPos(overlay_pos);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.55f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));

    ImGui::BeginChild(
        "##tree_child",
        ImVec2(w, h),
        true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings
    );

    // Track whether the overlay child window is hovered. If it is NOT hovered,
    // we want mouse events to pass through to the view underneath. We achieve
    // this by not consuming any input when the mouse is outside.
    //
    // ImGui::IsWindowHovered with RootAndChildWindows catches hovers on nested
    // children (like the scrolling region inside).
    bool overlay_hovered = ImGui::IsWindowHovered(
        ImGuiHoveredFlags_RootAndChildWindows |
        ImGuiHoveredFlags_AllowWhenBlockedByActiveItem
    );

    ImGui::BeginGroup();

    ImGui::TextUnformatted("Tree");
    ImGui::Separator();

    draw_filter_field();
    ImGui::Separator();

    draw_tree_nodes(root);

    ImGui::EndGroup();

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopID();

    ImGui::SetCursorScreenPos(saved_cursor);

    // If the overlay is NOT hovered, tell ImGui to clear any hover on this
    // child so the parent window (the view) can receive mouse events.
    if (!overlay_hovered) {
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
    }

    current_tree = nullptr;
    return dirty;
}

void TreeOverlay::draw_filter_field() {
    const ImGuiStyle& style = ImGui::GetStyle();

    float text_w = ImGui::CalcTextSize(filter_buf).x;
    float field_w =
        std::max(250.0f, text_w + 2.0f * style.FramePadding.x + 12.0f);

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Filter:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(field_w);

    bool has_error = filter_error.has_value();

    if (has_error) {
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

    if (ImGui::InputText("##filter", filter_buf, IM_ARRAYSIZE(filter_buf))) {
        dirty = true;
    }

    if (has_error) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", filter_error->c_str());
        }

        // Draw error indicator "!" inside the text field
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 rmin = ImGui::GetItemRectMin();
        ImVec2 rmax = ImGui::GetItemRectMax();

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
}

void TreeOverlay::draw_tree_nodes(
    Node* root
) {
    TreePath pth("/");
    draw_node(root, "/", 0, false, pth);
}

void TreeOverlay::draw_node(
    Node* node,
    const std::string& label,
    int depth,
    bool parent_dimmed,
    TreePath& full_path
) {
    (void)depth;
    ImGui::PushID(node);

    if (filter_path.has_value()) {
        node->glob_matches = full_path.matches_glob(filter_path.value());
    } else {
        node->glob_matches = std::nullopt;
    }

    const bool has_children = !node->children.empty();
    const ImGuiTreeNodeFlags base_flags =
        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_FramePadding |
        (has_children
             ? ImGuiTreeNodeFlags(0)
             : ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);

    const bool dimmed = parent_dimmed || !node->checked;
    bool dimmed_here = !parent_dimmed && !node->checked;
    if (dimmed_here) {
        float base_alpha = ImGui::GetStyle().Alpha;
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, base_alpha * 0.45f);
    }

    if (ImGui::Checkbox("##visible", &node->checked)) {
        dirty = true;
    }
    ImGui::SameLine(0.0f, 4.0f);

    bool open = false;
    if (has_children) {
        open = ImGui::TreeNodeEx(label.c_str(), base_flags);
        tree_open[node] = open;
    } else {
        tree_open[node] = false;
        ImGui::TreeNodeEx(label.c_str(), base_flags);
    }

    // Draw glob match indicator
    if (node->glob_matches.has_value()) {
        const bool match = node->glob_matches.value_or(false);

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
        for (const auto& c : node->children) {
            full_path.components.push_back(c.first);
            draw_node(c.second.get(), c.first, depth + 1, dimmed, full_path);
            full_path.components.pop_back();
        }
        ImGui::TreePop();
    }

    if (dimmed_here) {
        ImGui::PopStyleVar();
    }
    ImGui::PopID();
}

float TreeOverlay::compute_tree_content_height(
    Node* root
) const {
    float row_h = ImGui::GetFrameHeightWithSpacing();

    int count = 0;
    std::function<void(Node*)> walk = [&](Node* n) {
        count++;
        auto it = tree_open.find(n);
        bool is_open = (it != tree_open.end()) ? it->second : false;
        if (!n->children.empty() && is_open) {
            for (auto& kv : n->children) {
                walk(kv.second.get());
            }
        }
    };

    walk(root);
    return count * row_h;
}

float TreeOverlay::compute_filter_row_width() const {
    const ImGuiStyle& style = ImGui::GetStyle();
    float label_w = ImGui::CalcTextSize("Filter:").x;
    float text_w = ImGui::CalcTextSize(filter_buf).x;
    float field_w =
        std::max(250.0f, text_w + 2.0f * style.FramePadding.x + 12.0f);
    return label_w + style.ItemSpacing.x + field_w;
}

float TreeOverlay::compute_tree_content_width(
    Node* root
) const {
    const ImGuiStyle& style = ImGui::GetStyle();
    const float indent = style.IndentSpacing;
    const float row_h = ImGui::GetFrameHeight();
    const float chk_pad = style.ItemInnerSpacing.x;
    const float label_pad = ImGui::GetTreeNodeToLabelSpacing();
    const float badge_w = 12.0f;

    float max_w = 0.0f;

    std::function<void(Node*, const char*, int)> walk =
        [&](Node* n, const char* label, int depth) {
            bool has_children = !n->children.empty();
            float left = depth * indent + row_h + chk_pad +
                         (has_children ? row_h : 0.0f) + label_pad;

            float total = left + ImGui::CalcTextSize(label).x + badge_w;
            if (total > max_w) {
                max_w = total;
            }

            auto it = tree_open.find(n);
            bool is_open = (it != tree_open.end()) ? it->second : false;
            if (has_children && is_open) {
                for (auto& kv : n->children) {
                    walk(kv.second.get(), kv.first.c_str(), depth + 1);
                }
            }
        };

    walk(root, "/", 0);
    return max_w;
}

}  // namespace slamd
