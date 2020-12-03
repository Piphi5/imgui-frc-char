// MIT License

#pragma once

#include <future>
#include <memory>
#include <string>

#include <portable-file-dialogs.h>
#include <wpi/Error.h>
#include <wpi/SmallVector.h>

#include "backend/ProjectCreator.h"

namespace frcchar {
/**
 * The generator is responsible for asking the user about the parameters of the
 * mechanism they are trying to characterize. It will then generate the
 * appropriate robot project and deploy it to the roboRIO.
 */
class Generator {
 public:
  /**
   * Initializes this instance of the Generator window and adds it to the main
   * FRC Characterization GUI window.
   */
  void Initialize();

 private:
  /**
   * Selects the folder where the project will be stored.
   */
  void SelectProjectLocation();

  /**
   * Generates the project based on the parameters provided.
   */
  void GenerateProject();

  /**
   * Deploys the generated project to a roboRIO. The project is simulated if the
   * team number is zero.
   */
  void DeployProject();

  /**
   * Checks if the project generation has completed (the generation runs on a
   * separate thread).
   */
  LLVM_NODISCARD bool IsGenerationReady() const;

  static const char* kProjectTypes[];
  static const char* kGyros[];
  static const char* kMotorControllers[];

  int* m_teamNumber = nullptr;
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

  std::unique_ptr<ProjectCreator> m_creator;
  std::string m_deployOutput = "";

  std::future<void> m_generationStatus;
  std::future<void> m_deployStatus;
  std::unique_ptr<pfd::select_folder> m_folderSelector;
};
}  // namespace frcchar
