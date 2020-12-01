// MIT License

#pragma once

#if defined(__GNUG__) && !defined(__clang__) && __GNUC__ < 8
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <wpi/Error.h>

#include <exception>
#include <string>
#include <utility>

namespace frcchar {
class ProjectCreator {
 public:
  ProjectCreator(const std::string &dir, const std::string &name,
                 const int &team);

  void CreateProject();
  void DeployProject(std::string *result);

 private:
  const std::string &m_dir;
  const std::string &m_name;
  const int &m_team;

  LLVM_NODISCARD fs::path GetProjectPath() const;
};
}  // namespace frcchar
