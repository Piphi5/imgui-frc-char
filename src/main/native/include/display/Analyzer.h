// MIT License

#pragma once

#include <memory>
#include <string>

#include <portable-file-dialogs.h>

#include "backend/DataProcessor.h"

namespace frcchar {
/**
 * The analyzer GUI takes care of data analysis, including calculation of
 * feedforward and feedback gains.
 */
class Analyzer {
 public:
  /**
   * Initializes this instance of the Analyzer window and adds it to the main
   * FRC Characterization GUI window.
   */
  void Initialize();

 private:
  /**
   * Opens the data from the specified JSON.
   */
  void OpenData();

  std::unique_ptr<pfd::open_file> m_fileOpener;
  std::string m_fileLocation;
  std::string m_modifiedLocation;

  std::unique_ptr<DataProcessor> m_processor;

  DataProcessor::FFGains m_ffGains{0_V, 0_V / 1_mps, 0_V / 1_mps_sq, 0.0};
  DataProcessor::FBGains m_fbGains{0.0, 0.0};
  DataProcessor::GainPreset m_preset{true, 20_ms, 0_s, 1 / 1_V, true};
  DataProcessor::LQRParameters m_params{1_m, 1.5_mps, 7_V};
};
}  // namespace frcchar
