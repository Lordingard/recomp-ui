#ifdef NDEBUG
#undef NDEBUG
#endif
#include "recomp_runtime_ui.h"
#include "recomp_runtime_ui_internal.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace {

int get_value(void *context, const RecompRuntimeUiItem *, int *out) {
    *out = *static_cast<int *>(context);
    return 1;
}

int set_value(void *context, const RecompRuntimeUiItem *, int value) {
    *static_cast<int *>(context) = value;
    return 1;
}

void render_case(uint32_t presentation_flags, ImVec2 display,
                 float minimum_width, float minimum_height,
                 float maximum_width, bool long_integer = false) {
    static const char *const modes[] = { "Standard (4:3)", "16:9", "Adaptive" };
    static const RecompRuntimeUiItem items[] = {
        { "view", "Display", "View mode", "Choose the visible game area.",
          RECOMP_RUNTIME_UI_CHOICE, 0, 2, 1, modes, 3, nullptr },
    };
    static const RecompRuntimeUiItem slot_items[] = {
        { "system.state_slot", "Assist Tools", "State slot",
          "Slot used by Save state and Load state (slots 1-9 also have function-key shortcuts).",
          RECOMP_RUNTIME_UI_INT, 1, 9, 1, nullptr, 0, nullptr },
    };
    int value = 0;
    RecompRuntimeUiConfig config{};
    config.title = "Runtime UI";
    config.subtitle = "NINTENDO 64";
    config.items = long_integer ? slot_items : items;
    config.item_count = 1;
    config.theme = "n64";
    config.callbacks.context = &value;
    config.callbacks.get_value = get_value;
    config.callbacks.set_value = set_value;
    config.presentation_flags = presentation_flags;

    RecompRuntimeUi *ui = recomp_runtime_ui_create(&config);
    assert(ui != nullptr);
    recomp_runtime_ui_open(ui);
    if (long_integer) recomp_runtime_ui_enter_section(ui, 0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = display;
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char *font_pixels = nullptr;
    int font_w = 0, font_h = 0;
    io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_w, &font_h);

    ImGui::NewFrame();
    recomp_runtime_ui_render_imgui(ui);
    ImGuiWindow *window = ImGui::FindWindowByName("##recomp-runtime-ui");
    assert(window != nullptr);
    std::fprintf(stderr, "runtime-ui test display=%.0fx%.0f window=%.0fx%.0f\n",
                 display.x, display.y, window->Size.x, window->Size.y);
    assert(window->Size.x >= minimum_width);
    assert(window->Size.y >= minimum_height);
    assert(window->Size.x <= maximum_width);
    const float first_frame_font_scale = window->FontWindowScale;
    ImGui::Render();
    assert(ImGui::GetDrawData()->CmdListsCount > 0);
    if (long_integer) {
        bool found_wrapped_text = false;
        const ImDrawData* data = ImGui::GetDrawData();
        for (int list = 0; list < data->CmdListsCount; ++list) {
            for (const ImDrawCmd& command : data->CmdLists[list]->CmdBuffer) {
                const float clip_width = command.ClipRect.z - command.ClipRect.x;
                const float clip_height = command.ClipRect.w - command.ClipRect.y;
                if (command.ElemCount > 0 && clip_width > 20.0f &&
                    clip_width < 380.0f && clip_height > 40.0f && clip_height < 200.0f)
                    found_wrapped_text = true;
            }
        }
        assert(found_wrapped_text && "long row text must have its own bounded, multiline column");
    }

    ImGui::NewFrame();
    recomp_runtime_ui_render_imgui(ui);
    window = ImGui::FindWindowByName("##recomp-runtime-ui");
    assert(window != nullptr);
    assert(std::fabs(window->FontWindowScale - first_frame_font_scale) <
           0.001f);
    ImGui::Render();

    ImGui::DestroyContext();
    recomp_runtime_ui_destroy(ui);
}

} // namespace

int main() {
    render_case(0, ImVec2(1280.0f, 720.0f), 779.0f, 671.0f, 781.0f);
    render_case(RECOMP_RUNTIME_UI_PRESENTATION_TOUCH_FRIENDLY,
                ImVec2(3088.0f, 1440.0f), 3000.0f, 1300.0f, 3020.0f);
    render_case(0, ImVec2(1200.0f, 800.0f), 779.0f, 679.0f, 781.0f, true);
    render_case(0, ImVec2(720.0f, 480.0f), 671.0f, 431.0f, 673.0f, true);
    return 0;
}
