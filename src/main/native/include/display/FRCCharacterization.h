// MIT License

#pragma once

#include <functional>

namespace frcchar {
class FRCCharacterization {
 public:
  static void GlobalInit();
  static bool Initialize();
  static void Main();
  static void Exit();

  enum WindowVisibility { kHide = 0, kShow, kDisabled };

  static void AddMainMenu(std::function<void()> menu);
  static void AddOptionMenu(std::function<void()> menu);

  static void Add(std::function<void()> init);
  static void AddWindow(const char* name, std::function<void()> window);
};
}  // namespace frcchar
