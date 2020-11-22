// MIT License

#include "AnalyzeData.h"

#include <imgui.h>
#include <portable-file-dialogs.h>
#include <wpigui.h>

#include <future>
#include <iostream>
#include <thread>

#include <wpi/StringRef.h>
#include <wpi/json.h>
#include <wpi/raw_istream.h>

#include "FRCCharacterizationGUI.h"

using namespace frcchar;

static std::unique_ptr<pfd::open_file> gFileOpener;

static void LoadJSON() {
  if (gFileOpener && gFileOpener->ready()) {
    auto result = gFileOpener->result();
    if (!result.empty()) {
      if (wpi::StringRef(result[0]).endswith(".json")) {
        std::error_code ec;
        wpi::raw_fd_istream f(result[0], ec);
        if (ec) {
          std::cout << "Unable to open JSON" << std::endl;
          return;
        } else {
          wpi::json j;
          try {
            j = wpi::json::parse(f);
            auto a =
                j.at("slow-forward").get<std::vector<std::array<double, 9>>>();

            for (auto&& aa : a) {
              for (auto&& b : aa) {
                std::cout << b << ", " << std::endl;
              }
              std::cout << std::endl;
            }

            std::cout << "Parsed" << std::endl;
          } catch (const wpi::json::parse_error& e) {
            std::cout << e.what() << std::endl;
          }
        }
      }
    }
    gFileOpener.reset();
  }
}

void AnalyzeData::Initialize() {
  FRCCharacterizationGUI::AddWindow("Analyze Data", [] {
    if (ImGui::Button("Choose JSON File")) {
      gFileOpener = std::make_unique<pfd::open_file>("Choose JSON");
    }
    LoadJSON();
  });
}
