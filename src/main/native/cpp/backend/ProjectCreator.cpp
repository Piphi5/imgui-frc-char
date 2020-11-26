// MIT License

#include "backend/ProjectCreator.h"

#if defined(__GNUG__) && !defined(__clang__) && __GNUC__ < 8
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <filesystem>
#include <system_error>

#include <wpi/json.h>
#include <wpi/raw_ostream.h>

#include "generated/BuildGradle.h"

using namespace frcchar;

void ProjectCreator::CreateProject(const std::string& dir,
                                   const std::string& name, int team) {
  // Check if the project directory exists.
  std::filesystem::path p = dir + (dir.back() == fs::path::preferred_separator
                                       ? name
                                       : fs::path::preferred_separator + name);
  bool exists = fs::exists(p);

  auto p1 = p;
  p1.append(".wpilib");

  // Create the directory if the project directory does not exist.
  if (!exists) {
    fs::create_directory(p);
    fs::create_directory(p1);
  }

  // Create the build.gradle.
  std::error_code ec;
  wpi::raw_fd_ostream gradle(p.string() + "/build.gradle", ec);
  gradle << kBuildGradleContents << "\n";
  gradle.close();

  // Create the .wpilib/wpilib_preferences.json
  wpi::raw_fd_ostream properties(
      p.string() + "/.wpilib/wpilib_preferences.json", ec);
  wpi::json preferences;
  preferences["enableCppIntellisense"] = false;
  preferences["currentLanguage"] = "java";
  preferences["projectYear"] = "2021";
  preferences["teamNumber"] = team;
  properties << preferences.dump() << "\n";
  properties.close();

  if (ec) wpi::outs() << ec.message() << "\n";
}
