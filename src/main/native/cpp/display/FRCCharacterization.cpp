// MIT License

#include "display/FRCCharacterization.h"

#include <wpigui.h>

using namespace frcchar;

glass::WindowManager FRCCharacterization::Manager("frc-char");
glass::MainMenuBar FRCCharacterization::MenuBar;

void FRCCharacterization::GlobalInit() {
  Manager.GlobalInit();
  wpi::gui::AddLateExecute([] { MenuBar.Display(); });
}
