#pragma once

#include <wpi/SmallVector.h>

namespace frcchar {
class Generator {
 public:
  void Initialize();

 private:
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

  bool m_useIntegratedSensor = true;
  bool m_useNEOSensor = true;
};
}  // namespace frcchar