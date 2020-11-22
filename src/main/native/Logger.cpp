

#include "Logger.h"

#include <imgui.h>
#include <networktables/NetworkTableInstance.h>

#include <array>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include "FRCCharacterizationGUI.h"

using namespace frcchar;

std::string Logger::gProjectType = "Drivetrain";
static bool gAddTimestamp = true;
static int gTeamNumber = 0;

static float gQuasistaticRampVoltage = 0.25f;
static float gDynamicStepVoltage = 6.0f;
static float gRotationVoltage = 2.0f;

static std::future<bool> gNotificationFuture;
static bool gConnectionEstablished = false;
static bool gConnectionStatus = false;

struct TestState {
  enum RunState { kNever, kInProgress, kFinished };
  std::string name;
  RunState runState;
};

static const char* gOpenedPopup = "";

static void CreateTestButton(const char* text, bool connection, bool run) {
  ImGui::SetNextItemWidth(70);
  if (connection) {
    if (ImGui::Button(text)) {
      ImGui::OpenPopup("Warning");
      gOpenedPopup = text;
    }
    if (gOpenedPopup == text && ImGui::BeginPopupModal("Warning")) {
      ImGui::Text(
          "Please enable the robot in autonomous mode, and then disable it "
          "before it runs out of space. \n Note: The robot will continue to "
          "move until you disable it - It is your responsibility to ensure it "
          "does not hit anything!");
      if (ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
        gOpenedPopup = "";
      }

      ImGui::EndPopup();
    }
  } else {
    ImGui::TextDisabled("%s", text);
  }
  ImGui::SameLine(200);
  ImGui::Text(run ? "Run" : "Not Run");
}

void Logger::Initialize() {
  FRCCharacterizationGUI::AddWindow("Logger", [] {
    // Add test type.
    ImGui::Text("%s", ("Project Type: " + gProjectType).c_str());

    // Add timestamp.
    ImGui::Checkbox("Timestamp // Team: ", &gAddTimestamp);

    ImGui::SameLine(210);

    ImGui::SetNextItemWidth(70);
    ImGui::InputInt("##label", &gTeamNumber, 0);
    ImGui::SameLine(300);
    if (!gNotificationFuture.valid() && !gConnectionEstablished) {
      if (ImGui::Button("Connect")) {
        gNotificationFuture = std::async(std::launch::async, [] {
          if (gTeamNumber != 0) {
            nt::NetworkTableInstance::GetDefault().StartClientTeam(gTeamNumber);
          } else {
            nt::NetworkTableInstance::GetDefault().StartClient("localhost");
          }
          bool connected = false;
          auto start = std::chrono::system_clock::now();

          while (!connected && (std::chrono::system_clock::now() - start) <
                                   std::chrono::seconds(3)) {
            connected = nt::NetworkTableInstance::GetDefault().IsConnected();
          }

          return connected;
        });
      }
    } else if (!gConnectionEstablished &&
               gNotificationFuture.wait_for(std::chrono::seconds(0)) ==
                   std::future_status::timeout) {
      ImGui::Button("Connecting...");
    } else if (gNotificationFuture.valid()) {
      gConnectionStatus = gNotificationFuture.get();
      gConnectionEstablished = true;
      if (!gConnectionStatus)
        nt::NetworkTableInstance::GetDefault().StopClient();
    }

    if (gConnectionEstablished) {
      ImGui::Button(gConnectionStatus ? "Connected" : "Connection Failed");
      ImGui::SameLine();
      if (ImGui::Button("Reset")) {
        gConnectionStatus = false;
        gConnectionEstablished = false;
      }
    }

    // Separation.
    ImGui::Separator();
    ImGui::Spacing();

    // Add voltage parameters
    ImGui::Text("Voltage Parameters");
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("Quasistatic Ramp Rate (V/s)", &gQuasistaticRampVoltage);
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("Dynamic Step Voltage (V)", &gDynamicStepVoltage);
    ImGui::SetNextItemWidth(70);
    ImGui::InputFloat("Rotation Voltage (V)", &gRotationVoltage);

    // Separation.
    ImGui::Separator();
    ImGui::Spacing();

    // Tests
    ImGui::Text("Tests");
    CreateTestButton("Quasistatic Forward", gConnectionStatus, false);
    CreateTestButton("Quasistatic Reverse", gConnectionStatus, false);
    CreateTestButton("Dynamic Forward", gConnectionStatus, false);
    CreateTestButton("Dynamic Reverse", gConnectionStatus, false);
    if (gProjectType == "Drivetrain")
      CreateTestButton("Track Width", gConnectionStatus, false);
  });
}

void Logger::UpdateProjectType(const std::string& type) { gProjectType = type; }
