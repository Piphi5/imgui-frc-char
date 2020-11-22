// MIT License

#include <wpigui.h>

#include <cstdlib>

#include "AnalyzeData.h"
#include "DataAnalyzer.h"
#include "FRCCharacterizationGUI.h"
#include "Logger.h"
#include "Parameters.h"

using namespace frcchar;

int main() {
  FRCCharacterizationGUI::GlobalInit();

#ifdef MACOSX_BUNDLE
  wpi::gui::AddInit([] {
    char* path = std::getenv("HOME");
    std::strcat(path, "/Library/Preferences/imgui-frc-char.ini");
    ImGui::GetIO().IniFilename = path;
  });
#endif

  Logger logger;

  FRCCharacterizationGUI::Add(AnalyzeData::Initialize);
  FRCCharacterizationGUI::Add(Parameters::Initialize);
  FRCCharacterizationGUI::Add([&logger] { logger.Initialize(); });

  if (!FRCCharacterizationGUI::Initialize()) return 0;

  FRCCharacterizationGUI::Main();
  FRCCharacterizationGUI::Exit();
}
