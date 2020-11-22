// MIT License

#include "DataAnalyzer.h"

#include <cmath>

using namespace frcchar;

const std::array<std::string, 4> DataAnalyzer::kJsonKeys = {
    "slow-forward", "slow-backward", "fast-forward", "fast-backward"};

const DataAnalyzer::FeedbackGainPreset DataAnalyzer::kDefault = {
    "Default",
    12.0,
    20_ms,
    true,
    DataAnalyzer::FeedbackGainPreset::ControllerType::kOnboard,
    0_s};
const DataAnalyzer::FeedbackGainPreset DataAnalyzer::kWPILib2020 = {
    "WPILib (2020-)",
    12,
    20_ms,
    true,
    DataAnalyzer::FeedbackGainPreset::ControllerType::kOnboard,
    0_s};
const DataAnalyzer::FeedbackGainPreset DataAnalyzer::kWPILib2019 = {
    "WPILIb (Pre-2020)",
    1,
    50_ms,
    false,
    DataAnalyzer::FeedbackGainPreset::ControllerType::kOnboard,
    0_s};

// https://phoenix-documentation.readthedocs.io/en/latest/ch14_MCSensor.html#changing-velocity-measurement-parameters
// 100 ms sampling period + a moving average window size of 64 (i.e. a 64-tap
// FIR) = 100/2 ms + (64-1)/2 ms = 81.5 ms.
// See above for more info on moving average delays.
const DataAnalyzer::FeedbackGainPreset DataAnalyzer::kTalonFX = {
    "Talon FX",
    1,
    1_ms,
    true,
    DataAnalyzer::FeedbackGainPreset::ControllerType::kTalon,
    81.5_ms};

// https://phoenix-documentation.readthedocs.io/en/latest/ch14_MCSensor.html#changing-velocity-measurement-parameters
// 100 ms sampling period + a moving average window size of 64 (i.e. a 64-tap
// FIR) = 100/2 ms + (64-1)/2 ms = 81.5 ms.
// See above for more info on moving average delays.
const DataAnalyzer::FeedbackGainPreset DataAnalyzer::kTalonSRX2020 = {
    "Talon SRX (2020-)",
    1,
    1_ms,
    true,
    DataAnalyzer::FeedbackGainPreset::ControllerType::kTalon,
    81.5_ms};

// https://phoenix-documentation.readthedocs.io/en/latest/ch14_MCSensor.html#changing-velocity-measurement-parameters
// 100 ms sampling period + a moving average window size of 64 (i.e. a 64-tap
// FIR) = 100/2 ms + (64-1)/2 ms = 81.5 ms.
// See above for more info on moving average delays.
const DataAnalyzer::FeedbackGainPreset DataAnalyzer::kTalonSRX2019 = {
    "Talon SRX (Pre-2020)",
    1023,
    1_ms,
    false,
    DataAnalyzer::FeedbackGainPreset::ControllerType::kTalon,
    81.5_ms};

// According to a REV employee on the FRC Discord the window size is 40 so delay
// = (40-1)/2 ms = 19.5 ms. See above for more info on moving average delays.
const DataAnalyzer::FeedbackGainPreset DataAnalyzer::kSparkMaxBrushless = {
    "Spark MAX (Brushless)",
    1,
    1_ms,
    false,
    DataAnalyzer::FeedbackGainPreset::ControllerType::kSpark,
    19.5_ms};

// https://www.revrobotics.com/content/sw/max/sw-docs/cpp/classrev_1_1_c_a_n_encoder.html#a7e6ce792bc0c0558fb944771df572e6a
// 64-tap FIR = (64-1)/2 ms = 31.5 ms delay.
// See above for more info on moving average delays.
const DataAnalyzer::FeedbackGainPreset DataAnalyzer::kSparkMaxBrushed = {
    "Spark MAX (Brushed)",
    1,
    1_ms,
    false,
    DataAnalyzer::FeedbackGainPreset::FeedbackGainPreset::ControllerType::
        kSpark,
    31.5_ms};

std::vector<std::array<double, 9>> DataAnalyzer::TrimQuasistaticTestData(
    std::vector<std::array<double, 9>>* data) const {
  if (m_test == Test::kDrivetrain) {
    std::remove_if(
        data->begin(), data->end(), [&](const std::array<double, 9>& arr) {
          return (arr[JSONIndices::kLEncoderVel] < m_motionThreshold) ||
                 (arr[JSONIndices::kLVolts] < 0) ||
                 (arr[JSONIndices::kREncoderVel] < m_motionThreshold) ||
                 (arr[JSONIndices::kRVolts] < 0);
        });
  } else {
    std::remove_if(
        data->begin(), data->end(), [&](const std::array<double, 9>& arr) {
          return (arr[JSONIndices::kLEncoderVel] < m_motionThreshold) ||
                 (arr[JSONIndices::kLVolts] < 0);
        });
  }
}

std::vector<std::array<double, 5>> DataAnalyzer::PrepareDrivetrainData() const {
  for (const auto& key : kJsonKeys) {
    auto data = m_json.at(key).get<std::vector<std::array<double, 9>>>();
    for (auto& entry : data) {
      entry[JSONIndices::kLVolts] = std::copysign(
          entry[JSONIndices::kLVolts], entry[JSONIndices::kLEncoderVel]);
      entry[JSONIndices::kRVolts] = std::copysign(
          entry[JSONIndices::kRVolts], entry[JSONIndices::kREncoderVel]);

      entry[JSONIndices::kLEncoderVel] *= m_unitsPerRotation;
      entry[JSONIndices::kREncoderVel] *= m_unitsPerRotation;

      entry[JSONIndices::kLEncoderPos] *= m_unitsPerRotation;
      entry[JSONIndices::kREncoderPos] *= m_unitsPerRotation;
    }
    TrimQuasistaticTestData(&data);
  }
}
