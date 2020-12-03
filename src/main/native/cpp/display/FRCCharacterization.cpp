// MIT License

#include "display/FRCCharacterization.h"

#include <wpigui.h>

#include "display/Analyzer.h"
#include "display/Generator.h"
#include "display/Logger.h"

using namespace frcchar;

glass::WindowManager FRCCharacterization::Manager("frc-char");
glass::MainMenuBar FRCCharacterization::MenuBar;

std::unique_ptr<Logger> FRCCharacterization::Logger =
    std::make_unique<frcchar::Logger>();
std::unique_ptr<Analyzer> FRCCharacterization::Analyzer =
    std::make_unique<frcchar::Analyzer>();
std::unique_ptr<Generator> FRCCharacterization::Generator =
    std::make_unique<frcchar::Generator>();

void FRCCharacterization::GlobalInit() {
  Manager.GlobalInit();

  // Add all of our widgets.
  wpi::gui::AddInit([] {
    Logger->Initialize();
    Analyzer->Initialize();
    Generator->Initialize();
  });

  // Add the main menu bar.
  wpi::gui::AddLateExecute([] { MenuBar.Display(); });
}
