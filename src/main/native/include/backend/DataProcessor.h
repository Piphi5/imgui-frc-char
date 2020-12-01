// MIT License

#pragma once

#include <array>
#include <string>
#include <tuple>
#include <vector>

#include <units/base.h>
#include <units/acceleration.h>
#include <units/dimensionless.h>
#include <units/time.h>
#include <units/velocity.h>
#include <units/voltage.h>

namespace units {
using Kv_t = decltype(1_V / 1_mps);
using Ka_t = decltype(1_V / 1_mps_sq);
}  // namespace units

namespace frcchar {
/**
 * This class is responsible for processing raw data from the data logger to
 * produce feedforward and feedback gains.
 */
class DataProcessor {
 public:
  /**
   * A struct that represents the feedforward gains produced by the analysis.
   * This includes the standard Ks, Kv, and Ka, along with a coefficient of
   * determination for the OLS multiple regression fit.
   */
  struct FFGains {
    units::volt_t Ks;
    units::Kv_t Kv;
    units::Ka_t Ka;
    double CoD;
  };

  /**
   * A struct that represents the feedback gains produced by the analysis. This
   * includes Kp and Kd. Kd will always be zero for a velocity loop type.
   */
  struct FBGains {
    double Kp, Kd;
  };

  /**
   * A struct that represents the presets for the feedback gains. These include
   * whether we want to calculate gains for a velocity loop, the nominal period
   * between controller updates, the latency in the sensor measurements, the
   * conversion from volts to the proprietary output and whether the controller
   * Kd is time normalized.
   */
  struct GainPreset {
    bool velocity;
    units::second_t dt, latency;
    units::unit_t<units::inverse<units::volt>> output;
    bool normalized;
  };

  /**
   * A struct that represents other settings for calculating feedforward and
   * feedback gains, including the output units per rotation of the output, the
   * maximum allowed excursion in the states and the maximum allowable control
   * effort.
   */
  struct Settings {
    units::meter_t unitsPerRotation;
    units::meter_t qp;
    units::meters_per_second_t qv;
    units::volt_t maxEffort;
  };

  /**
   * Constructs a new DataProcessor instance with the given gain preset.
   *
   * @param preset The preset to construct this processor instance with.
   */
  DataProcessor(const GainPreset& preset) : m_preset(preset) {}

  /**
   * Calculates the feedforward and feedback gains for the drivetrain, given a
   * path to the stored JSON.
   *
   * @param path The path to the JSON containing the raw data.
   *
   * @return 2 sets of feedforward and feedback gains, representing the left and
   *         right sides of the drivetrain respectively.
   */
  std::tuple<FFGains, FFGains, FBGains, FBGains> CalculateDrivetrainParameters(
      const std::string& path);

  /**
   * Calculates the feedforward and feedback gains for a mechanism, given a path
   * to the stored JSON.
   *
   * @param path The path to the JSON containing the raw data.
   *
   * @return A set of feedforward and feedback gains.
   */
  std::tuple<FFGains, FBGains> CalculateParameters(const std::string& path);

 private:
  using RawData = std::vector<std::array<double, 10>>;
  std::pair<std::vector<double>, std::vector<double>> PrepareDrivetrainData(
      const std::vector<std::array<double, 10>>& data);
  std::vector<double> PrepareData(std::vector<std::array<double, 10>>* data);

  std::vector<double> ComputeAcceleration(
      std::vector<std::array<double, 10>>* data);
  void TrimQuasistaticData(std::vector<std::array<double, 10>>* data);
  void TrimStepVoltageData(std::vector<double>* data);

  FFGains CalculateFeedforwardGains(const std::vector<double>& data);

  FBGains CalculatePositionFeedbackGains(const FFGains& ff, units::meter_t qp,
                                         units::meters_per_second_t qv,
                                         units::volt_t maxEffort,
                                         units::second_t period,
                                         units::second_t positionDelay);
  FBGains CalculateVelocityFeedbackGains(const FFGains& ff,
                                         units::meters_per_second_t qv,
                                         units::volt_t maxEffort,
                                         units::second_t period,
                                         units::second_t sensorDelay);

 private:
  GainPreset m_preset;
  Settings m_settings{0.58_m, 5_m, 7_mps, 12_V};
  static constexpr units::meters_per_second_t kQuasistaticVelocityThreshold =
      0.1_mps;
};
}  // namespace frcchar
