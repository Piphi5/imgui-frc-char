// MIT License

#pragma once

#include <glass/MainMenuBar.h>
#include <glass/WindowManager.h>

#include <memory>

namespace frcchar {

class Logger;
class Analyzer;
class Generator;

class FRCCharacterization {
 public:
  /**
   * Initializes the window manager for the program and adds the main menu bar.
   */
  static void GlobalInit();

  static glass::WindowManager Manager;
  static glass::MainMenuBar MenuBar;

  static std::unique_ptr<Logger> Logger;
  static std::unique_ptr<Analyzer> Analyzer;
  static std::unique_ptr<Generator> Generator;
};
}  // namespace frcchar
