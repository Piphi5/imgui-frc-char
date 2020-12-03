// MIT License

#pragma once

#include <array>
#include <string>
#include <tuple>
#include <vector>

#include <units/acceleration.h>
#include <units/base.h>
#include <units/dimensionless.h>
#include <units/time.h>
#include <units/velocity.h>
#include <units/voltage.h>
#include <wpi/StringMap.h>

namespace units {
using Kv_t = decltype(1_V / 1_mps);
using Ka_t = decltype(1_V / 1_mps_sq);
}  // namespace units

namespace frcchar {
/**
 * This class is responsible for processing raw data from the data logger to
 * produce feedforward and feedback gains. Each instance of this class
 * represents one JSON.
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
   * A struct that represents parameters for the LQR used to calculate feedback
   * gains.
   */
  struct LQRParameters {
    units::meter_t qp;
    units::meters_per_second_t qv;
    units::volt_t maxEffort;
  };

  /**
   * Constructs a new DataProcessor instance with the given gain preset.
   *
   * @param preset The preset to construct this processor instance with.
   */
  DataProcessor(std::string* path, FFGains* ffGains, FBGains* fbGains,
                GainPreset* preset, LQRParameters* params);

  /**
   * Calculates the feedback and feedforward gains given the current state of
   * this instance. This should be called whenever a value inside the gain
   * preset or LQR parameters has changed.
   */
  void Update();

 private:
  using RawData = std::vector<std::array<double, 10>>;

  /**
   * Trims quasistatic test data to eliminate data points where the velocity was
   * below the motion threshold or when the applied voltage was zero.
   */
  void TrimQuasistaticData(RawData* data);

  /**
   * Calculates acceleration by taking the slope of the secant line between
   * three data points. This data is then bundled with the other voltage and
   * velocity data.
   */
  std::vector<double> PrepareDataForAnalysis(RawData* data);

  /**
   * Trims acceleration data to remove all data points before the maximum
   * acceleration point.
   */
  void TrimStepVoltageData(std::vector<double>* data);

  /**
   * Calculates feedforward gains for the given data set.
   */
  void CalculateFeedforwardGains();

  /**
   * Calculates position feedback gains for the given data set using the
   * feedforward parameters.
   */
  void CalculatePositionFeedbackGains();

  /**
   * Calculates velocity feedback gains for the given data set using the
   * feedforward parameters.
   */
  void CalculateVelocityFeedbackGains();

  // Location of the JSON file.
  std::string& m_path;

  // Storage for feedforward and feedback gains.
  FFGains& m_ffGains;
  FBGains& m_fbGains;

  // Other values from the JSON.
  units::meter_t m_factor;
  std::string m_projectType;

  // Preset and LQR parameters.
  GainPreset& m_preset;
  LQRParameters& m_lqrParams;

  // Used to store the various data sets.
  wpi::StringMap<std::vector<double>> m_data;

  // Which dataset to use
  std::string m_dataset = "Backward";

  // Motion threshold.
  static constexpr auto kQuasistaticVelocityThreshold = 0.1_mps;
};
}  // namespace frcchar
