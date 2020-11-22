

#pragma once

#include <string>

namespace frcchar {
class Logger {
 public:
  static void Initialize();
  static void UpdateProjectType(const std::string& type);

 private:
  static std::string gProjectType;
};
}  // namespace frcchar
