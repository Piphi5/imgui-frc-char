// MIT License

#include "display/FRCCharacterization.h"

#include <wpigui.h>

#include "display/Analyzer.h"
#include "display/Generator.h"
#include "display/Logger.h"

using namespace frcchar;

glass::WindowManager FRCCharacterization::Manager("frc-char");
glass::MainMenuBar FRCCharacterization::MenuBar;

std::unique_ptr<Logger> FRCCharacterization::LoggerGUI =
    std::make_unique<Logger>();
std::unique_ptr<Analyzer> FRCCharacterization::AnalyzerGUI =
    std::make_unique<Analyzer>();
std::unique_ptr<Generator> FRCCharacterization::GeneratorGUI =
    std::make_unique<Generator>();

void FRCCharacterization::GlobalInit() {
  Manager.GlobalInit();

  // Add all of our widgets.
  wpi::gui::AddInit([] {
    LoggerGUI->Initialize();
    AnalyzerGUI->Initialize();
    GeneratorGUI->Initialize();
  });

  // Add the main menu bar.
  wpi::gui::AddLateExecute([] { MenuBar.Display(); });
}
