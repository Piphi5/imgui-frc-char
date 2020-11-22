// MIT License

#include <wpigui.h>

#include <cstdlib>

#include "AnalyzeData.h"
#include "DataAnalyzer.h"
#include "FRCCharacterizationGUI.h"
#include "Logger.h"
#include "Parameters.h"

int main() {
  frcchar::FRCCharacterizationGUI::GlobalInit();

#ifdef MACOSX_BUNDLE
  wpi::gui::AddInit([] {
    char* path = std::getenv("HOME");
    std::strcat(path, "/Library/Preferences/imgui-frc-char.ini");
    ImGui::GetIO().IniFilename = path;
  });
#endif

  frcchar::FRCCharacterizationGUI::Add(frcchar::AnalyzeData::Initialize);
  frcchar::FRCCharacterizationGUI::Add(frcchar::Parameters::Initialize);
  frcchar::FRCCharacterizationGUI::Add(frcchar::Logger::Initialize);

  if (!frcchar::FRCCharacterizationGUI::Initialize()) return 0;

  frcchar::FRCCharacterizationGUI::Main();
  frcchar::FRCCharacterizationGUI::Exit();
}
