// MIT License

#include "display/Logger.h"

#include <imgui.h>
#include <networktables/NetworkTableInstance.h>
#include <wpi/raw_ostream.h>

#include <array>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include "display/FRCCharacterization.h"

using namespace frcchar;

void Logger::Initialize() {
  // Add a new window to the GUI.
  FRCCharacterization::AddWindow("Logger", [&] {
    // Get the current width of the window. This will be used to scale our UI
    // elements.
    float width = ImGui::GetContentRegionAvail().x;

    // Display information about the test type.
    ImGui::Text("%s", ("Project Type: " + m_projectType).c_str());

    // Add NT connection buttons and team number input.
    // If the future is not valid or there is no need to reset, then it means we
    // need to show a button to establish a connection.
    if (!m_ntConnectionStatus.valid() && !m_ntNeedsReset) {
      if (ImGui::Button("Connect")) AttemptNTConnection();
    } else if (!m_ntNeedsReset && !IsNTConnectionStatusReady()) {
      // If the future is still calculating, then we need to display the
      // "Connecting..." text.
      ImGui::Button("Connecting...");
    } else if (m_ntConnectionStatus.valid()) {
      // Now that we have a state, we can show the reset button.
      m_ntNeedsReset = true;

      // Set the last NT connection to whatever the state was.
      m_lastNTConnection = m_ntConnectionStatus.get();
    }

    // If we need to show the reset button, it means that we either have a
    // failure or success state. We need to display this.
    if (m_ntNeedsReset) {
      ImGui::Button(m_lastNTConnection ? "Connected" : "Connection Failed");
      ImGui::SameLine();
      if (ImGui::Button("Reset")) {
        // Reset the statuses.
        m_ntNeedsReset = false;
        m_lastNTConnection = false;
      }
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(width / 5);
    ImGui::InputInt("Team Number", &m_teamNumber, 0);

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
      if (m_lastNTConnection) {
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
              "Please enable the robot in autonomous mode, and then disable it "
              "before it runs out of space. \n Note: The robot will continue "
              "to move until you disable it - It is your responsibility to "
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

    if (m_exception) {
      auto ex = m_exception;
      m_exception = std::exception_ptr();
      std::rethrow_exception(ex);
    }
  });
}

void Logger::UpdateProjectType(const std::string& type) {
  m_projectType = type;
}

void Logger::AttemptNTConnection() {
  m_ntConnectionStatus = std::async(std::launch::async, [&] {
    using std::chrono::system_clock;
    using std::chrono::seconds;

    bool connected = false;

    try {
      // Get the current time.
      auto startTime = system_clock::now();

      // Attempt connection.
      if (m_teamNumber == 0)
        nt::NetworkTableInstance::GetDefault().StartClient("localhost");
      else
        nt::NetworkTableInstance::GetDefault().StartClientTeam(m_teamNumber);

      // Wait for connection success or 3 seconds of connection failure.
      while (!(connected || system_clock::now() - startTime > seconds(3)))
        // Check whether a connection has been established.
        connected = nt::NetworkTableInstance::GetDefault().IsConnected();

      // If there was no connection established, we can stop attempting to
      // connect.
      if (!connected) {
        nt::NetworkTableInstance::GetDefault().StopClient();
        throw std::runtime_error("Connection was not established!");
      }
    } catch (const std::exception& e) {
      m_exception = std::current_exception();
    }

    // Return the connection status.
    return connected;
  });
}

bool Logger::IsNTConnectionStatusReady() const {
  return m_ntConnectionStatus.wait_for(std::chrono::seconds(0)) !=
         std::future_status::timeout;
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
void Logger::CreateDataFile() {
  m_fileCreationStatus = std::async(std::launch::async, [&] {
    try {
      throw std::runtime_error("This is not implemented yet.");
    } catch (const std::exception& e) {
      m_exception = std::current_exception();
    }
  });
}
