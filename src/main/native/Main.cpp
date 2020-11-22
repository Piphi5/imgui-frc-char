

#include <wpigui.h>

#include "AnalyzeData.h"
#include "FRCCharacterizationGUI.h"
#include "Logger.h"
#include "Parameters.h"

int main() {
  frcchar::FRCCharacterizationGUI::GlobalInit();

  frcchar::FRCCharacterizationGUI::Add(frcchar::AnalyzeData::Initialize);
  frcchar::FRCCharacterizationGUI::Add(frcchar::Parameters::Initialize);
  frcchar::FRCCharacterizationGUI::Add(frcchar::Logger::Initialize);

  if (!frcchar::FRCCharacterizationGUI::Initialize()) return 0;

  frcchar::FRCCharacterizationGUI::Main();
  frcchar::FRCCharacterizationGUI::Exit();
}
