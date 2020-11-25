// MIT License

#pragma once

#include <future>
#include <string>

namespace frcchar {
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
   * Attempt an NT connection and store it in a future.
   */
  void AttemptNTConnection();

  /**
   * Returns whether the NT connection status future is ready with a shared
   * state.
   */
  bool IsNTConnectionStatusReady() const;

 private:
  // NT Connection State
  std::future<bool> m_ntConnectionStatus;
  bool m_ntNeedsReset = false;
  bool m_lastNTConnection = false;

  // Project Settings
  std::string m_projectType = "Drivetrain";
  int m_teamNumber = 0;

  // Voltage Settings
  float m_quasistaticRampVoltage = 0.25f;
  float m_dynamicStepVoltage = 6.0f;
  float m_rotationVoltage = 2.0f;

  // Modal Popup Storage
  const char* m_openedPopup = "";
};
}  // namespace frcchar
