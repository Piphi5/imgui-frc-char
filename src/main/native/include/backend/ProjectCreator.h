// MIT License

#pragma once

#if defined(__GNUG__) && !defined(__clang__) && __GNUC__ < 8
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <exception>
#include <string>
#include <utility>

namespace frcchar {
class ProjectCreator {
 public:
  ProjectCreator(std::string dir, std::string name, int team)
      : m_dir(std::move(dir)), m_name(std::move(name)), m_team(team) {}

  void CreateProject();
  void DeployProject(std::string* result);


 private:
  std::string m_dir;
  std::string m_name;
  int m_team;

  fs::path m_thisProject;
};
}  // namespace frcchar
