// MIT License

#pragma once

#include <string>

namespace frcchar {
class ProjectCreator {
 public:
  static void CreateProject(const std::string& dir, const std::string& name,
                            int team);
};
}  // namespace frcchar
