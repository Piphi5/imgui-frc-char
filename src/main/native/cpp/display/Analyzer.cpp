// MIT License

#include "display/Analyzer.h"

#include <future>

#include <imgui.h>
#include <wpi/raw_ostream.h>

#include "backend/DataProcessor.h"
#include "display/FRCCharacterization.h"

using namespace frcchar;

void Analyzer::Initialize() {
  FRCCharacterization::AddWindow("Analyzer", [&] {
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

    ImGui::InputDouble("Ks", &Ks, 0, 0, "%2.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputDouble("Kv", &Kv, 0, 0, "%2.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputDouble("Ka", &Ka, 0, 0, "%2.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputDouble("R-Squared", &Rs, 0, 0, "%2.3f",
                       ImGuiInputTextFlags_ReadOnly);

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Feedback Gains");

    ImGui::InputDouble("Kp", &Kp, 0, 0, "%2.3f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputDouble("Kd", &Kd, 0, 0, "%2.3f", ImGuiInputTextFlags_ReadOnly);

    // Handle exceptions from other threads.
    HandleExceptions();
  });
}

void Analyzer::HandleExceptions() {
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
    try {
      std::async(std::launch::async, [&] {
        DataProcessor p{
            DataProcessor::GainPreset{true, 20_ms, 0_s, 1 / 1_V, true}};
        auto [ff, fb] = p.CalculateParameters(m_fileLocation);
        Ks = ff.Ks.to<double>();
        Kv = ff.Kv.to<double>();
        Ka = ff.Ka.to<double>();
        Rs = ff.CoD;
        Kp = fb.Kp;
        Kd = fb.Kd;
      });
    } catch (const std::exception& e) {
      m_exception = std::current_exception();
    }
  }
}
