// MIT License

#pragma once

#include <glass/WindowManager.h>
#include <glass/MainMenuBar.h>

#include <functional>

namespace frcchar {
class FRCCharacterization {
 public:
  /**
   * Initializes the window manager for the program and adds the main menu bar.
   */
  static void GlobalInit();

  static glass::WindowManager Manager;
  static glass::MainMenuBar MenuBar;
};
}  // namespace frcchar
