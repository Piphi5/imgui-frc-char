// MIT License

#include "display/FRCCharacterization.h"

#include <wpigui.h>

#include <functional>
#include <vector>

#include <wpi/raw_ostream.h>

using namespace frcchar;

struct Window {
  const char* name;
  std::function<void()> display;

  ImGuiWindowFlags flags = 0;
  bool visible = true;
  bool enabled = true;

  ImGuiCond posCond = 0;
  ImGuiCond sizeCond = 0;

  ImVec2 pos;
  ImVec2 size;

  bool setPadding = false;
  ImVec2 padding;

  Window(const char* name, std::function<void()> display)
      : name(name), display(display) {}
};

static std::vector<Window> gWindows;
static std::vector<std::function<void()>> gMenus;
static std::vector<std::function<void()>> gOptionMenus;
static std::string gException = "";

void FRCCharacterization::AddMainMenu(std::function<void()> menu) {
  if (menu) gMenus.emplace_back(std::move(menu));
}

void FRCCharacterization::AddOptionMenu(std::function<void()> menu) {
  if (menu) gOptionMenus.emplace_back(std::move(menu));
}

void FRCCharacterization::GlobalInit() { wpi::gui::CreateContext(); }

bool FRCCharacterization::Initialize() {
  // Scale default window positions.
  wpi::gui::AddWindowScaler([](float windowScale) {
    for (auto&& window : gWindows) {
      if ((window.posCond & ImGuiCond_FirstUseEver) != 0) {
        window.pos.x *= windowScale;
        window.pos.y *= windowScale;
        window.size.x += windowScale;
        window.size.y += windowScale;
      }
    }
  });

  wpi::gui::AddLateExecute([] {
    ImGui::BeginMainMenuBar();
    wpi::gui::EmitViewMenu();
    ImGui::EndMainMenuBar();

    for (auto&& window : gWindows) {
      try {
        if (ImGui::Begin(window.name)) {
          window.display();
        }
      } catch (const std::exception& e) {
        gException = e.what();
        ImGui::OpenPopup("Exception!");
      }
      if (ImGui::BeginPopupModal("Exception!")) {
        ImGui::Text("%s", gException.c_str());
        if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
      }
      ImGui::End();
    }
  });

  return wpi::gui::Initialize("FRC Characterization", 1280, 720);
}

void FRCCharacterization::Main() { wpi::gui::Main(); }
void FRCCharacterization::Exit() { wpi::gui::Exit(); }

void FRCCharacterization::Add(std::function<void()> initialize) {
  wpi::gui::AddInit(std::move(initialize));
}

void FRCCharacterization::AddWindow(const char* name,
                                    std::function<void()> window) {
  gWindows.emplace_back(name, window);
}
