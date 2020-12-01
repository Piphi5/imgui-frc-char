// MIT License

#pragma once

#include <portable-file-dialogs.h>

#include <exception>
#include <memory>
#include <string>

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

  double Ks = 0.0;
  double Kv = 0.0;
  double Ka = 0.0;
  double Rs = 0.0;
  
  double Kp = 0.0;
  double Kd = 0.0;
  
  std::exception_ptr m_exception;
};
}  // namespace frcchar
