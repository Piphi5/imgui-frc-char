// MIT License

#pragma once

#include <array>
#include <string>

#include <units/time.h>
#include <wpi/json.h>

namespace frcchar {
class DataAnalyzer {
 public:
  struct FeedforwardParameters {
    double ks;
    double kv;
    double ka;
    double kcos;
    double rsquared;
  };

  struct FeedbackParameters {
    double kp;
    double kd;
  };

  struct FeedbackGainPreset {
    enum ControllerType { kOnboard, kTalon, kSpark };

    std::string name;
    double maxControllerOutput;
    units::second_t period;
    bool timeNormalized;
    ControllerType type;
    units::second_t measurementDelay;
  };

  FeedforwardParameters RunDriveAnalysis() const;
  FeedforwardParameters RunElevatorAnalysis() const;
  FeedforwardParameters RunArmAnalysis() const;
  FeedforwardParameters RunSimpleAnalysis() const;

  double CalculateTrackWidth() const;
  FeedbackParameters CalculateGains() const;

  std::vector<std::array<double, 5>> PrepareDrivetrainData() const;
  std::vector<std::array<double, 9>> TrimQuasistaticTestData(
      std::vector<std::array<double, 9>>* data) const;

 private:
  struct JSONIndices {
    static constexpr int kTime = 0;
    static constexpr int kBattery = 1;
    static constexpr int kLVolts = 2;
    static constexpr int kRVolts = 3;
    static constexpr int kLEncoderPos = 4;
    static constexpr int kREncoderPos = 5;
    static constexpr int kLEncoderVel = 6;
    static constexpr int kREncoderVel = 7;
    static constexpr int kGyro = 8;
  };

  struct PreparedDataIndices {
    static constexpr int kTime = 0;
    static constexpr int kVelocity = 1;
    static constexpr int kPosition = 2;
    static constexpr int kAcceleration = 3;
    static constexpr int kCos = 4;
  };

  enum Test { kDrivetrain, kSimple, kElevator, kArm };

  static const std::array<std::string, 4> kJsonKeys;

  static const FeedbackGainPreset kDefault;
  static const FeedbackGainPreset kWPILib2020;
  static const FeedbackGainPreset kWPILib2019;
  static const FeedbackGainPreset kTalonFX;
  static const FeedbackGainPreset kTalonSRX2020;
  static const FeedbackGainPreset kTalonSRX2019;
  static const FeedbackGainPreset kSparkMaxBrushless;
  static const FeedbackGainPreset kSparkMaxBrushed;

  double m_unitsPerRotation;
  wpi::json m_json;
  Test m_test;
  double m_motionThreshold;
};
}  // namespace frcchar
