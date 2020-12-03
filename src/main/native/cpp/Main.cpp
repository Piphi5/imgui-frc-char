// MIT License

#include <glass/Context.h>
#include <wpigui.h>

#include <wpi/json.h>
#include <wpi/raw_istream.h>
#include <wpi/raw_ostream.h>

#include "backend/DataProcessor.h"
#include "display/Analyzer.h"
#include "display/FRCCharacterization.h"
#include "display/Generator.h"
#include "display/Logger.h"

using namespace frcchar;

int main() {
  // Create the Dear ImGui context and the Glass context.
  wpi::gui::CreateContext();
  glass::CreateContext();

  // Initialize FRC Characterization.
  FRCCharacterization::GlobalInit();

  // Add the list of widgets to the menu bar.
  FRCCharacterization::MenuBar.AddMainMenu([] {
    if (ImGui::BeginMenu("Widgets")) {
      FRCCharacterization::Manager.DisplayMenu();
      ImGui::EndMenu();
    }
  });

  // Start the Dear ImGui application.
  wpi::gui::Initialize("FRC Characterization", 1280, 720);

  // Run the main loop.
  wpi::gui::Main();

  // Destroy contexts and exit.
  glass::DestroyContext();
  wpi::gui::DestroyContext();
}
