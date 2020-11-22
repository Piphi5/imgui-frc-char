// MIT License

#pragma once

#include <functional>

namespace frcchar {
class FRCCharacterizationGUI {
 public:
  static void GlobalInit();
  static bool Initialize();
  static void Main();
  static void Exit();

  enum WindowVisibility { kHide = 0, kShow, kDisabled };

  static void AddMainMenu(std::function<void()> menu);
  static void AddOptionMenu(std::function<void()> menu);
  static void SetWindowVisibility(const char* name,
                                  WindowVisibility visibility);
  static void SetDefaultWindowPos(const char* name, float x, float y);
  static void SetDefaultWindowSize(const char* name, float width, float height);

  static void Add(std::function<void()> init);
  static void AddWindow(const char* name, std::function<void()> window);
};
}  // namespace frcchar
