// MIT License

#include "display/Analyzer.h"

#include <imgui.h>

#include <wpi/raw_ostream.h>

#include "display/FRCCharacterization.h"

using namespace frcchar;

void Analyzer::Initialize() {
  FRCCharacterization::AddWindow("Analyzer", [&] {
    // Handle exceptions from other threads.
    HandleExceptions();
  });
}

void Analyzer::HandleExceptions() {
  // Get the current width of the window. This will be used to scale the UI
  // elements.
  float width = ImGui::GetContentRegionAvail().x;

  // Create section for file selection.
  ImGui::Text("File Selection");

  // Scale the font size down to fit more of the path.
  ImFont f = *ImGui::GetFont();
  f.Scale *= 0.85;
  ImGui::PushFont(&f);

  ImGui::SetNextItemWidth(width / 1.5);
  ImGui::InputText("##label", const_cast<char*>(m_modifiedLocation.c_str()),
                   m_modifiedLocation.capacity() + 1,
                   ImGuiInputTextFlags_ReadOnly);
  ImGui::PopFont();
  ImGui::SameLine();

  // Create button to select folder location.
  if (ImGui::Button("Choose..")) {
    m_fileOpener = std::make_unique<pfd::open_file>("Select Data JSON");
  }
  OpenData();

  if (m_exception) {
    auto ex = m_exception;
    m_exception = std::exception_ptr();
    std::rethrow_exception(ex);
  }
}
void Analyzer::OpenData() {
  if (m_fileOpener && m_fileOpener->ready(0)) {
    m_fileLocation = m_fileOpener->result().at(0);
    m_modifiedLocation = m_fileLocation;

    const char* home = std::getenv("HOME");
    if (home) {
      size_t len = std::strlen(home);
      bool trailingSlash = home[len - 1] == '/';
      size_t index = m_modifiedLocation.find(home);
      if (index != std::string::npos)
        m_modifiedLocation.replace(index, len, trailingSlash ? "~/" : "~");
    }

    m_fileOpener.reset();
  }
}
