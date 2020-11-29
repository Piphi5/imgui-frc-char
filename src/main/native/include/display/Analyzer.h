#pragma once

#include <exception>
#include <memory>
#include <string>

#include <portable-file-dialogs.h>

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

  std::exception_ptr m_exception;
};
}  // namespace frcchar