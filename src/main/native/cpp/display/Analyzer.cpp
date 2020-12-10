// MIT License

#include "display/Analyzer.h"

#include <implot.h>

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
    ImGui::SameLine(width / 2);
    ImGui::SetNextItemWidth(width / 3);
    if (ImGui::Combo("##datatype", &m_dataType, DataProcessor::kDataSources,
                     IM_ARRAYSIZE(DataProcessor::kDataSources)) &&
        m_processor)
      m_processor->Update();

    auto showGain = [&](double* source, const char* name) {
      ImGui::SetNextItemWidth(width / 8);
      ImGui::InputDouble(name, source, 0, 0, "%2.3f",
                         ImGuiInputTextFlags_ReadOnly);
    };

    // Display feedforward gains and r-squared for fit.
    showGain(reinterpret_cast<double*>(&m_ffGains.Ks), "Ks");
    showGain(reinterpret_cast<double*>(&m_ffGains.Kv), "Kv");
    ImGui::SameLine(width / 2);
    if (ImGui::Button("Voltage-Domain Plots")) {
      ImPlot::FitNextPlotAxes();
      ImGui::OpenPopup("Voltage-Domain Plots");
    }

    if (ImGui::BeginPopupModal("Voltage-Domain Plots")) {
      if (ImPlot::BeginPlot("Voltage-Domain Plots")) {
        auto& data = m_processor->GetData();
        std::vector<ImPlotPoint> points;
        for (size_t i = 0; i < data.size(); i += 4) {
          points.emplace_back(data[i] - m_ffGains.Ks.to<double>() -
                                  m_ffGains.Ka.to<double>() * data[i + 3],
                              data[i + 2]);
        }

        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 1,
                                   ImVec4(0, 1, 0, 0.5f), IMPLOT_AUTO);
        ImPlot::PlotScatter("Velocity-Portion Voltage", points.data(),
                            points.size());
        ImPlot::EndPlot();
      }

      if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    showGain(reinterpret_cast<double*>(&m_ffGains.Ka), "Ka");
    showGain(&m_ffGains.CoD, "R-Squared");

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Feedback Gains");

    // Display feedback gains.
    showGain(&m_fbGains.Kp, "Kp");
    showGain(&m_fbGains.Kd, "Kd");
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
    m_processor =
        std::make_unique<DataProcessor>(&m_fileLocation, &m_ffGains, &m_fbGains,
                                        &m_preset, &m_params, &m_dataType);
    m_processor->Update();

    m_fileOpener.reset();
  }
}
