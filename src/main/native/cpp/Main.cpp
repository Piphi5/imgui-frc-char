// MIT License

#include <wpigui.h>

#include <cstdlib>

#include "FRCCharacterizationGUI.h"
#include "Generator.h"
#include "Logger.h"

using namespace frcchar;

int main() {
  FRCCharacterizationGUI::GlobalInit();

  Logger logger;
  Generator generator;

  FRCCharacterizationGUI::Add([&generator] { generator.Initialize(); });
  FRCCharacterizationGUI::Add([&logger] { logger.Initialize(); });

  if (!FRCCharacterizationGUI::Initialize()) return 0;

  FRCCharacterizationGUI::Main();
  FRCCharacterizationGUI::Exit();
}
