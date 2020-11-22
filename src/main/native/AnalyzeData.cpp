

#include "AnalyzeData.h"

#include <imgui.h>
#include <wpigui.h>

#include <future>
#include <iostream>
#include <thread>

#include "FRCCharacterizationGUI.h"

using namespace frcchar;

struct Result {
  const char* name = "A";
};

static std::future<Result> gCalculatedFuture;

template <typename R>
bool is_ready(std::future<R> const& f) {
  return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

void AnalyzeData::Initialize() {
  FRCCharacterizationGUI::AddWindow("Analyze Data", [] {
    if (!gCalculatedFuture.valid()) {
      if (ImGui::Button("Calculate")) {
        gCalculatedFuture = std::async(std::launch::async, [] {
          std::this_thread::sleep_for(std::chrono::seconds(2));
          return Result();
        });
      }
      return;
    }

    if (is_ready(gCalculatedFuture)) {
      // gCalculatedFuture.get();
      // gCalculatedFuture = std::future<Result>();
      ImGui::Button("Calculated!");
    } else {
      ImGui::Button("Calculating...");
    }
  });
}
