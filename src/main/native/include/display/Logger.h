// MIT License

#pragma once

#include <future>
#include <memory>
#include <string>

#include <portable-file-dialogs.h>

namespace frcchar {
/**
 * The logger GUI takes care of running the characterization tests over
 * NetworkTables and logging the data. This data is then stored in a JSON file
 * which can be used for analysis.
 */
class Logger {
 public:
  /**
   * Initializes this instance of the Logger window and adds it to the main FRC
   * Characterization GUI window.
   */
  void Initialize();

  /**
   * Updates the project type (i.e. drivetrain, elevator, arm, simple).
   */
  void UpdateProjectType(const std::string& type);

 private:
  /**
   * Selects the folder where the JSON file will be saved.
   */
  void SelectDataFolder();

  /**
   * Creates the data file from the logged data.
   */
  void CreateDataFile();

 private:
  // NT Connection State
  bool m_ntNeedsReset = true;
  bool m_ntConnectionStatus = false;

  // Project Settings
  std::string m_projectType = "Drivetrain";
  int* m_teamNumber = nullptr;
  std::unique_ptr<pfd::select_folder> m_folderSelector;

  // Folder locations for the JSON files.
  std::string m_fileLocation;
  std::string m_modifiedLocation;

  // Voltage Settings
  float m_quasistaticRampVoltage = 0.25f;
  float m_dynamicStepVoltage = 6.0f;
  float m_rotationVoltage = 2.0f;

  // Modal Popup Storage
  const char* m_openedPopup = "";
};
}  // namespace frcchar
