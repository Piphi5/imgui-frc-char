// MIT License

#pragma once

#include <portable-file-dialogs.h>

#include <exception>
#include <memory>
#include <string>

#include "backend/DataProcessor.h"

namespace frcchar {
class Analyzer {
 public:
  void Initialize();

 private:
  void HandleExceptions();
  void OpenData();

  std::unique_ptr<pfd::open_file> m_fileOpener;
  std::string m_fileLocation;
  std::string m_modifiedLocation;

  DataProcessor::FFGains m_ffGains{0_V, 0_V / 1_mps, 0_V / 1_mps_sq, 0.0};
  DataProcessor::FBGains m_fbGains{0.0, 0.0};
  DataProcessor::GainPreset m_preset{true, 20_ms, 0_s, 1 / 1_V, true};
  DataProcessor::LQRParameters m_params{1_m, 1.5_mps, 7_V};

  std::exception_ptr m_exception;
};
}  // namespace frcchar
