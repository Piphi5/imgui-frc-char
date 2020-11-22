

#include "Parameters.h"

#include <imgui.h>

#include <future>

#include "FRCCharacterizationGUI.h"
#include "Logger.h"

using namespace frcchar;

// The various project types that we support.
const char* gProjectTypes[] = {"Drivetrain", "Arm", "Elevator", "Simple"};

// The various control types (i.e. motor controllers).
const char* gControlTypes[] = {"Spark MAX (Brushed)", "Spark MAX (Brushless)",
                               "CTRE", "Simple"};

const char* gMotorControllers[] = {
    "Spark",        "Victor",        "VictorSP",    "PWMTalonSRX",
    "WPI_TalonSRX", "WPI_VictorSPX", "WPI_TalonFX", "CANSparkMax"};

const char* gGyros[] = {"NavX", "Pigeon", "ADXRS450", "AnalogGyro", "None"};

// Global variables to store data.
static int gTeamNumber = 0;
static int gProjectType = 0;

static int gEncoderPorts[3] = {0, -1, -1};
static int gRightEncoderPorts[3] = {1, -1, -1};

static bool gMotorsInverted = false;
static bool gRightMotorsInverted = true;

static int gControlType = 3;
static int gMotorController = 1;
static int gEncoderEPR = 1;

static int gGyro = 0;
static char gGyroPort[40] = "SPI.Port.kMXP";

static bool gUseIntegratedNEO = true;

static std::future<void> gProjectGenFuture;

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

void Parameters::Initialize() {
  // Add this window to the main GUI.
  FRCCharacterizationGUI::AddWindow("Parameters", [] {
    int w = ImGui::GetContentRegionAvail().x * 0.95;

    ImGui::InputInt("Team Number", &gTeamNumber);

    ImGui::Combo("Project Type", &gProjectType, gProjectTypes,
                 IM_ARRAYSIZE(gProjectTypes));

    ImGui::Spacing();

    if (gProjectType == 0) {
      ImGui::InputInt3("Left Encoder Ports", gEncoderPorts);
      ImGui::SameLine(w);
      HelpMarker("The left encoder ports");
      ImGui::InputInt3("Right Encoder Ports", gRightEncoderPorts);
      ImGui::SameLine(w);
      HelpMarker("");

    } else {
      ImGui::InputInt3("Encoder Ports", gEncoderPorts);
    }

    ImGui::Spacing();

    if (gProjectType == 0) {
      ImGui::Checkbox("Left Motors Inverted", &gMotorsInverted);
      ImGui::Checkbox("Right Motors Inverted", &gRightMotorsInverted);
    } else {
      ImGui::Checkbox("Motors Inverted", &gMotorsInverted);
    }

    ImGui::Combo("Control Type", &gControlType, gControlTypes,
                 IM_ARRAYSIZE(gControlTypes));

    ImGui::Combo("Motor Controller Class", &gMotorController, gMotorControllers,
                 IM_ARRAYSIZE(gMotorControllers));
    ImGui::SameLine(w);
    HelpMarker(
        "These represent class names for the motor controllers on your robot.");

    ImGui::Spacing();

    ImGui::InputInt("Encoder EPR", &gEncoderEPR);
    ImGui::SameLine(w);
    HelpMarker(
        "The encoder edges per revolution. This is not the same as cycles per "
        "revolution. This value should be the edges per revolution of the "
        "wheels and so should take into account the gearing between the "
        "encoder and the wheels.");

    ImGui::Spacing();

    ImGui::Combo("Gyro", &gGyro, gGyros, IM_ARRAYSIZE(gGyros));
    ImGui::InputText("Gyro Port", gGyroPort, IM_ARRAYSIZE(gGyroPort));
    ImGui::SameLine(w);
    HelpMarker("This is whatever you put into the constructor of your Gyro.");

    ImGui::Spacing();

    if (gMotorController == 7) {
      ImGui::Checkbox("Integrated NEO Encoder", &gUseIntegratedNEO);
      ImGui::SameLine(w);
      HelpMarker(
          "Whether you want to use the built-in NEO encoder or if you want to "
          "use a separate quadrature encoder connected to the Spark MAX.");
    }

    ImGui::Separator();
    ImGui::Spacing();

    if (!gProjectGenFuture.valid()) {
      if (ImGui::Button("Generate Project")) {
        gProjectGenFuture = std::async(std::launch::async, [] {
          std::this_thread::sleep_for(std::chrono::seconds(2));
        });
      }
    } else if (gProjectGenFuture.wait_for(std::chrono::seconds(0)) ==
               std::future_status::timeout) {
      ImGui::Button("Generating...");
    } else if (gProjectGenFuture.valid()) {
      ImGui::Button("Project Generated!");
      ImGui::SameLine();
      if (ImGui::Button("OK")) {
        static_cast<void>(gProjectGenFuture.get());
      }
    }
    Logger::UpdateProjectType(gProjectTypes[gProjectType]);
  });
}
