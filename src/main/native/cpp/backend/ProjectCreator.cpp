// MIT License

#include "backend/ProjectCreator.h"

#include <array>
#include <cstdio>
#include <system_error>

#include <wpi/json.h>
#include <wpi/raw_ostream.h>

using namespace frcchar;

ProjectCreator::ProjectCreator(const std::string& dir, const std::string& name,
                               const int& team)
    : m_dir(dir), m_name(name), m_team(team) {}

void ProjectCreator::CreateProject() {
  auto p = GetProjectPath();

  // Check if the project directory exists.
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
  gradle <<
#include "generated/BuildGradle.h"
      ;
  gradle.close();

  // Create the .wpilib/wpilib_preferences.json
  wpi::raw_fd_ostream properties(
      p.string() + "/.wpilib/wpilib_preferences.json", ec);
  wpi::json preferences;
  preferences["enableCppIntellisense"] = false;
  preferences["currentLanguage"] = "java";
  preferences["projectYear"] = "2021";
  preferences["teamNumber"] = m_team;
  properties << preferences.dump() << "\n";
  properties.close();

  if (ec) wpi::outs() << ec.message() << "\n";
}

void ProjectCreator::DeployProject(std::string* result) {
  std::string jdk = std::getenv("HOME");
  jdk += ((jdk.back() == fs::path::preferred_separator ? "" : "/")) +
         std::string("wpilib/2021/jdk");

  std::array<char, 128> buffer{};

  std::string command;
  if (m_team != 0) {
    command = "./gradlew deploy -Dorg.gradle.java.home=" + jdk + " 2>&1";
  } else {
    command = "./gradlew simulateJava -Dorg.gradle.java.home=" + jdk + " 2>&1";
  }

  auto pipe = popen(
      std::string("cd " + GetProjectPath().string() + ";" + command).c_str(),
      "r");
  if (!pipe) throw std::runtime_error("The command failed to execute.");

  while (!feof(pipe)) {
    if (std::fgets(buffer.data(), 128, pipe) != nullptr) {
      *result += buffer.data();
    }
  }

  pclose(pipe);
}

fs::path ProjectCreator::GetProjectPath() const {
  return m_dir + (m_dir.back() == fs::path::preferred_separator
                      ? m_name
                      : fs::path::preferred_separator + m_name);
}
