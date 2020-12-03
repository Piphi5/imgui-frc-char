// MIT License

#include "display/Logger.h"

#include <glass/Context.h>
#include <imgui.h>
#include <networktables/NetworkTableInstance.h>
#include <wpigui.h>

#include <array>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include <wpi/raw_ostream.h>

#include "display/FRCCharacterization.h"

using namespace frcchar;

void Logger::Initialize() {
  // Add an NT connection listener used to update the NetworkTables state.
  auto inst = nt::GetDefaultInstance();
  auto poller = nt::CreateConnectionListenerPoller(inst);
  nt::AddPolledConnectionListener(poller, true);
  wpi::gui::AddEarlyExecute(
      [poller, &m_ntConnectionStatus = m_ntConnectionStatus] {
        bool timedOut;
        for (auto&& event : nt::PollConnectionListener(poller, 0, &timedOut))
          m_ntConnectionStatus = event.connected;
      });

  m_teamNumber = glass::GetStorage().GetIntRef("LoggerTeam");

  // Add a new window to the GUI.
  glass::Window* window = FRCCharacterization::Manager.AddWindow("Logger", [&] {
    // Get the current width of the window. This will be used to scale
    // our UI elements.
    float width = ImGui::GetContentRegionAvail().x;

    // Display information about the test type.
    ImGui::Text("%s", ("Project Type: " + m_projectType).c_str());

    ImGui::SetNextItemWidth(width / 5);
    ImGui::InputInt("Team Number", m_teamNumber, 0);

    if (ImGui::Button("Apply")) {
      m_ntNeedsReset = true;
    }

    ImGui::SameLine();
    ImGui::Text(m_ntConnectionStatus ? "NT Connected" : "NT Disconnected");

    if (m_ntNeedsReset) {
      m_ntNeedsReset = false;
      nt::StopClient(nt::GetDefaultInstance());
      if (*m_teamNumber == 0) {
        nt::StartClient(nt::GetDefaultInstance(), "localhost", NT_DEFAULT_PORT);
      } else {
        nt::StartClientTeam(nt::GetDefaultInstance(), *m_teamNumber,
                            NT_DEFAULT_PORT);
      }
    }

    // Create new section for voltage parameters.
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Voltage Parameters");

    // Add input boxes for the various voltage parameters.
    auto createVoltageParameterInputs = [&](const char* name, float* data) {
      ImGui::SetNextItemWidth(width / 5);
      ImGui::InputFloat(name, data);
    };

    createVoltageParameterInputs("Quasistatic Ramp Rate (V/s)",
                                 &m_quasistaticRampVoltage);
    createVoltageParameterInputs("Dynamic Step Voltage (V)",
                                 &m_dynamicStepVoltage);
    if (m_projectType == "Drivetrain")
      createVoltageParameterInputs("Rotation Voltage (V)", &m_rotationVoltage);

    // Create new section for tests.
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Tests");

    // Add buttons and text for the tests.
    auto createTestButtons = [&](const char* name, bool run) {
      // Display buttons if we have an NT connection.
      if (m_ntConnectionStatus) {
        // Create button to run test.
        if (ImGui::Button(name)) {
          // Open the warning message.
          ImGui::OpenPopup("Warning");

          // Store the name of the button that caused the warning.
          m_openedPopup = name;
        }
        // Create modal window.
        if (m_openedPopup == name && ImGui::BeginPopupModal("Warning")) {
          // Show warning text.
          ImGui::Text(
              "Please enable the robot in autonomous mode, and then "
              "disable it "
              "before it runs out of space. \n Note: The robot will "
              "continue "
              "to move until you disable it - It is your "
              "responsibility to "
              "ensure it does not hit anything!");
          // Add "Close" button
          if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
          ImGui::EndPopup();
        }
      } else {
        // Show disabled text because there is no NT connection.
        ImGui::TextDisabled("%s", name);
      }

      // Show whether the tests were run or not.
      ImGui::SameLine(width * 0.7);
      ImGui::Text(run ? "Run" : "Not Run");
    };

    createTestButtons("Quasistatic Forward", false);
    createTestButtons("Quasistatic Reverse", false);
    createTestButtons("Dynamic Forward", false);
    createTestButtons("Dynamic Backward", false);
    if (m_projectType == "Drivetrain") createTestButtons("Track Width", false);

    // Create new section for file saving settings.
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Save Settings");

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

    if (ImGui::Button("Choose..."))
      m_folderSelector = std::make_unique<pfd::select_folder>("Select Folder");

    SelectDataFolder();

    ImGui::SameLine();
    if (ImGui::Button("Save")) CreateDataFile();
  });

  window->DisableRenamePopup();
  window->SetDefaultPos(541, 27);
  window->SetDefaultSize(355, 497);
}

void Logger::UpdateProjectType(const std::string& type) {
  m_projectType = type;
}

void Logger::SelectDataFolder() {
  if (m_folderSelector && m_folderSelector->ready()) {
    m_fileLocation = m_folderSelector->result();
    m_modifiedLocation = m_fileLocation;

    const char* home = std::getenv("HOME");
    if (home) {
      size_t len = std::strlen(home);
      bool trailingSlash = home[len - 1] == '/';
      size_t index = m_modifiedLocation.find(home);
      if (index != std::string::npos)
        m_modifiedLocation.replace(index, len, trailingSlash ? "~/" : "~");
    }

    m_folderSelector.reset();
  }
}

void Logger::CreateDataFile() {}
