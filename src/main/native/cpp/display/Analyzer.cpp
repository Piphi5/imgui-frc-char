// MIT License

#include "display/Analyzer.h"

#include <future>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <wpi/raw_ostream.h>

#include "backend/DataProcessor.h"
#include "display/FRCCharacterization.h"

using namespace frcchar;

void Analyzer::Initialize() {
  auto window = FRCCharacterization::Manager.AddWindow("Analyzer", [&] {
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

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Feedforward Gains");

    ImGui::InputDouble("Ks", reinterpret_cast<double*>(&(m_ffGains.Ks)), 0, 0,
                       "%2.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputDouble("Kv", reinterpret_cast<double*>(&(m_ffGains.Kv)), 0, 0,
                       "%2.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputDouble("Ka", reinterpret_cast<double*>(&(m_ffGains.Ka)), 0, 0,
                       "%2.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputDouble("R-Squared", &(m_ffGains.CoD), 0, 0, "%2.3f",
                       ImGuiInputTextFlags_ReadOnly);

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Feedback Gains");

    ImGui::InputDouble("Kp", &(m_fbGains.Kp), 0, 0, "%2.3f",
                       ImGuiInputTextFlags_ReadOnly);
    ImGui::InputDouble("Kd", &(m_fbGains.Kd), 0, 0, "%2.3f",
                       ImGuiInputTextFlags_ReadOnly);
  });

  window->DisableRenamePopup();
  window->SetDefaultPos(912, 27);
  window->SetDefaultSize(342, 497);
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

    m_processor.reset();
    m_processor = std::make_unique<DataProcessor>(
        &m_fileLocation, &m_ffGains, &m_fbGains, &m_preset, &m_params);
    m_processor->Update();

    m_fileOpener.reset();
  }
}
