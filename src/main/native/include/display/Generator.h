// MIT License

#pragma once

#include <portable-file-dialogs.h>

#include <exception>
#include <future>
#include <memory>
#include <string>

#include <wpi/Error.h>
#include <wpi/SmallVector.h>

#include "backend/ProjectCreator.h"

namespace frcchar {
class Generator {
 public:
  void Initialize();

 private:
  void SelectProjectLocation();
  void GenerateProject();
  void DeployProject();
  LLVM_NODISCARD bool IsGenerationReady() const;

  static const char* kProjectTypes[];
  static const char* kGyros[];
  static const char* kMotorControllers[];

  int m_teamNumber = 0;
  int m_projectType = 0;
  int m_motorController = 1;
  float m_encoderEPR = 256.0f;
  int m_gyro = 0;

  wpi::SmallVector<int, 2> m_leftMotorPorts;
  wpi::SmallVector<int, 2> m_rightMotorPorts;

  int m_leftEncoderPorts[2] = {0, 1};
  int m_rightEncoderPorts[2] = {2, 3};

  size_t m_motorPortsUsed = 1;

  char m_gyroPort[40] = "SPI.Port.kMXP";
  std::string m_projectName = "frc-characterization";
  std::string m_projectLocation = "Choose Location...";
  std::string m_modifiedLocation = "";

  bool m_useIntegratedSensor = true;
  bool m_useNEOSensor = true;

  std::exception_ptr m_exception;

  ProjectCreator m_creator{m_projectLocation, m_projectName, m_teamNumber};
  std::string m_deployOutput = "";

  std::future<void> m_generationStatus;
  std::future<void> m_deployStatus;
  std::unique_ptr<pfd::select_folder> m_folderSelector;
};
}  // namespace frcchar
