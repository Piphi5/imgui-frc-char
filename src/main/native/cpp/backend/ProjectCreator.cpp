#include "backend/ProjectCreator.h"

#include <filesystem>
#include <wpi/raw_ostream.h>
#include <system_error>

#include "generated/BuildGradle.h"

using namespace frcchar;

void ProjectCreator::CreateProject(const std::string& dir,
                                   const std::string& name) {
  // Check if the project directory exists.
  std::filesystem::path p =
      dir + (dir.back() == std::filesystem::path::preferred_separator
                 ? name
                 : std::filesystem::path::preferred_separator + name);
  bool exists = std::filesystem::exists(p);

  // Create the directory if the project directory does not exist.
  if (!exists) std::filesystem::create_directory(p);

  // Create a simple test file in our directory.
  std::error_code ec;
  wpi::raw_fd_ostream gradle(p.string() + "/build.gradle", ec);
  gradle << kBuildGradleContents << "\n";
  gradle.close();

  if (ec) wpi::outs() << ec.message() << "\n";
}
