// MIT License

#include "backend/ProjectCreator.h"

#include <array>
#include <cstdio>
#include <system_error>

#include <wpi/json.h>
#include <wpi/raw_ostream.h>

using namespace frcchar;

void ProjectCreator::CreateProject() {
  // Check if the project directory exists.
  m_thisProject = m_dir + (m_dir.back() == fs::path::preferred_separator
                               ? m_name
                               : fs::path::preferred_separator + m_name);
  //  bool exists = fs::exists(m_thisProject);
  //
  //  auto p1 = m_thisProject;
  //  p1.append(".wpilib");
  //
  //  // Create the directory if the project directory does not exist.
  //  if (!exists) {
  //    fs::create_directory(m_thisProject);
  //    fs::create_directory(p1);
  //  }
  //
  //  // Create the build.gradle.
  //  std::error_code ec;
  //  wpi::raw_fd_ostream gradle(m_thisProject.string() + "/build.gradle", ec);
  //  gradle <<
  //#include "generated/BuildGradle.h"
  //      ;
  //  gradle.close();
  //
  //  // Create the .wpilib/wpilib_preferences.json
  //  wpi::raw_fd_ostream properties(
  //      m_thisProject.string() + "/.wpilib/wpilib_preferences.json", ec);
  //  wpi::json preferences;
  //  preferences["enableCppIntellisense"] = false;
  //  preferences["currentLanguage"] = "java";
  //  preferences["projectYear"] = "2021";
  //  preferences["teamNumber"] = m_team;
  //  properties << preferences.dump() << "\n";
  //  properties.close();
  //
  //  if (ec) wpi::outs() << ec.message() << "\n";
}

void ProjectCreator::DeployProject(std::string* result) {
  std::array<char, 128> buffer{};

  m_thisProject = m_dir + (m_dir.back() == fs::path::preferred_separator
                               ? m_name
                               : fs::path::preferred_separator + m_name);
  static_cast<void>(m_team);

  auto pipe =
      popen(std::string("cd " + m_thisProject.string() + ";./gradlew deploy")
                .c_str(),
            "r");
  if (!pipe) throw std::runtime_error("The command failed to execute.");

  while (!feof(pipe)) {
    if (fgets(buffer.data(), 128, pipe) != nullptr) {
      *result += buffer.data();
    }
  }

  pclose(pipe);
//  if (rc != 0)
//    throw std::runtime_error("The error code was " + std::to_string(rc));
}
