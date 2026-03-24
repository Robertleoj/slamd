#pragma once
#include <imgui.h>

namespace slamd {

class ControlsHint {
   public:
    /// Renders a small controls hint in the bottom-right of the current ImGui
    /// window.
    void render();
};

}  // namespace slamd
