#pragma once

#include <string>
#include <queue>

namespace frcchar {
// Represents the type of test that will be conducted.
enum Type { kDrivetrain, kElevator, kArm, kSimple };

/**
 * Class that manages conducting the various tests (quasistatic, dynamic, track
 * width) using the NetworkTables API.
 */
class TestConductor {
 public:
  struct Error {
    bool error;
    std::string msg;
  };

  Error Conduct(const frcchar::Type& type);

 private:
  bool IsRobotConnected() const;
  bool IsRobotEnabled() const;
  void SetVoltage(double voltage);
};
}  // namespace frcchar
