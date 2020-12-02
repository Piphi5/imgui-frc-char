// MIT License

#include "display/Generator.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <cstdlib>

#include <wpi/raw_ostream.h>

#include "backend/ProjectCreator.h"
#include "display/FRCCharacterization.h"

using namespace frcchar;

const char* Generator::kProjectTypes[] = {"Drivetrain", "Arm", "Elevator",
                                          "Simple"};
const char* Generator::kGyros[] = {"NavX", "Pigeon", "ADXRS450", "AnalogGyro",
                                   "None"};
const char* Generator::kMotorControllers[] = {
    "Spark",        "Victor",        "VictorSP",    "PWMTalonSRX",
    "WPI_TalonSRX", "WPI_VictorSPX", "WPI_TalonFX", "CANSparkMax"};

void Generator::SelectProjectLocation() {
  if (m_folderSelector && m_folderSelector->ready()) {
    m_projectLocation = m_folderSelector->result();
    m_modifiedLocation = m_projectLocation;

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

void Generator::GenerateProject() {
  m_generationStatus = std::async(std::launch::async, [&] {
    try {
      m_creator.CreateProject();
    } catch (const std::exception& e) {
      m_exception = std::current_exception();
    }
  });
}

bool Generator::IsGenerationReady() const {
  return m_generationStatus.wait_for(std::chrono::seconds(0)) !=
         std::future_status::timeout;
}

void Generator::Initialize() {
  // Add a new window to the GUI.
  FRCCharacterization::Manager.AddWindow("Generator", [&] {
    // Get the current width of the window. This will be used to scale the UI
    // elements.
    int width = ImGui::GetContentRegionAvail().x;

    ImGui::Text("General");

    // Add team number input.
    ImGui::SetNextItemWidth(width / 5);
    ImGui::InputInt("Team", &m_teamNumber, 0);

    // Add project type input.
    ImGui::SetNextItemWidth(width / 2.5);
    ImGui::Combo("Project Type", &m_projectType, kProjectTypes,
                 IM_ARRAYSIZE(kProjectTypes));

    // Create a function that will add help tooltips beside certain UI widgets.
    auto createHelperMarker = [&width](const char* desc) {
      ImGui::SameLine(width * 0.85);
      ImGui::TextDisabled("(?)");
      if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
      }
    };

    // Create new section for motor information.
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Motor Information");

    // Add motor controller input.
    ImGui::SetNextItemWidth(width / 2.5);
    ImGui::Combo("Motor Controller", &m_motorController, kMotorControllers,
                 IM_ARRAYSIZE(kMotorControllers));
    createHelperMarker(
        "This represents the physical motor controller that is connected to "
        "the mechanism that you are trying to characterize.");

    // Add section for motor ports.
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Motor Ports");

    // Create inputs for motor ports.
    for (size_t i = 0; i < m_motorPortsUsed; ++i) {
      // Ensure vector sizes are correct. It's enough to check only one vector
      // since we always keep their sizes the same.
      if (i == m_leftMotorPorts.size()) {
        m_leftMotorPorts.emplace_back(0);
        m_rightMotorPorts.emplace_back(1);
      }

      // Create input field for motor ports (left for drivetrain.).
      ImGui::SetNextItemWidth(width / 8);
      ImGui::InputInt(
          m_projectType == 0
              ? ("L##" + std::to_string(i)).c_str()
              : (i == 0 ? "Leader" : ("Follower " + std::to_string(i)).c_str()),
          &m_leftMotorPorts[i], 0);

      // Create input fields for right motor ports if we have drivetrain
      // selected.
      if (m_projectType == 0) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(width / 8);
        ImGui::InputInt(
            (std::string("R ") +
             (i == 0 ? "Leader" : ("Follower " + std::to_string(i))))
                .c_str(),
            &m_rightMotorPorts[i], 0);
      }

      // Add minus button to remove extra ports.
      if (i == 0 && m_motorPortsUsed > 1) {
        ImGui::SameLine(width * 0.93);
        ImGui::Text("(-)");
        if (ImGui::IsItemClicked()) m_motorPortsUsed--;
      }

      // Add plus button to add extra ports.
      if (i == 0) {
        ImGui::SameLine(width * 0.85);
        ImGui::Text("(+)");
        if (ImGui::IsItemClicked()) m_motorPortsUsed++;
      }
    }

    // Check if we want to use the integrated sensor (for motor controllers that
    // support this).
    ImGui::Spacing();
    if (m_motorController > 3) {
      ImGui::Checkbox("Use Integrated Sensor", &m_useIntegratedSensor);
      createHelperMarker(
          "Whether you want to use the sensor connected directly to the motor "
          "controller. If unchecked, you will be asked to specify roboRIO "
          "encoder ports.");
    }

    // Check if we want to use the NEO sensor for CANSparkMAX.
    if (m_useIntegratedSensor && m_motorController == 7) {
      ImGui::Checkbox("Use NEO Encoder", &m_useNEOSensor);
      createHelperMarker(
          "Whether you want to use the sensor on the NEO / NEO 550 itself. If "
          "unchecked, an external quadrature encoder connected to the Spark "
          "MAX will be used.");
    }

    // Create new section for roboRIO encoders if we need them.
    if (!m_useIntegratedSensor || m_motorController <= 3) {
      ImGui::Separator();
      ImGui::Spacing();
      ImGui::Text("roboRIO Encoder Ports");

      ImGui::SetNextItemWidth(width / 3.4);
      ImGui::InputInt2(m_projectType == 0 ? "L DIO Ports" : "DIO Ports",
                       m_leftEncoderPorts);
      if (m_projectType == 0) {
        ImGui::SetNextItemWidth(width / 3.4);
        ImGui::InputInt2("R DIO Ports", m_rightEncoderPorts);
      }
    }

    // Create encoder EPR section.
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Units");

    ImGui::SetNextItemWidth(width / 5);
    ImGui::InputFloat("Encoder EPR", &m_encoderEPR, 0);
    createHelperMarker(
        "The encoder edges per revolution. This is not the same as cycles per "
        "revolution. This value should be the edges per revolution of the "
        "wheels and so should take into account the gearing between the "
        "encoder and the wheels.");

    // Create new section for gyro if we are performing drivetrain
    // characterization.
    if (m_projectType == 0) {
      ImGui::Separator();
      ImGui::Spacing();
      ImGui::Text("Gyroscope Information");

      // Add gyro selector.
      ImGui::SetNextItemWidth(width / 2.5);
      ImGui::Combo("Gyro", &m_gyro, kGyros, IM_ARRAYSIZE(kGyros));
      createHelperMarker(
          "This represents the physical gyroscope on your drivetrain.");

      // Add gyro port information.
      ImGui::SetNextItemWidth(width / 2.5);
      ImGui::InputText("Gyro Port", m_gyroPort, IM_ARRAYSIZE(m_gyroPort));
      createHelperMarker(
          "This represents what you would normally put inside the constructor "
          "of your gyro in your robot code.");
    }

    // Add section for project generation.
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Project Generation");

    ImGui::SetNextItemWidth(width * 0.5);
    ImGui::InputText("##labela", &m_projectName);

    ImGui::SetNextItemWidth(width * 0.5);

    // Scale the font size down to fit more of the path.
    ImFont f = *ImGui::GetFont();
    f.Scale *= 0.85;
    ImGui::PushFont(&f);

    ImGui::InputText("##labelb", const_cast<char*>(m_modifiedLocation.c_str()),
                     m_modifiedLocation.capacity() + 1,
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::PopFont();
    ImGui::SameLine();

    if (ImGui::Button("Choose..")) {
      m_folderSelector =
          std::make_unique<pfd::select_folder>("Select Project Location");
    }
    SelectProjectLocation();

    ImGui::SameLine();
    bool disabled = m_projectLocation == "Choose Location...";
    if (disabled) {
      ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.25f);
    }
    if (!m_generationStatus.valid()) {
      if (ImGui::Button("Generate")) GenerateProject();
    } else if (!IsGenerationReady()) {
      ImGui::Button("Generating...");
    } else if (m_generationStatus.valid()) {
      if (ImGui::Button("Generated!")) m_generationStatus.get();
    }

    if (disabled) {
      ImGui::PopItemFlag();
      ImGui::PopStyleVar();
    }

    if (ImGui::Button("Deploy Project")) {
      ImGui::OpenPopup("Deploy Status...");
      DeployProject();
    }

    auto size = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowSize(ImVec2(size.x / 2, size.y * 0.95));
    if (ImGui::BeginPopupModal("Deploy Status...")) {
      ImGui::Text("GradleRIO Output");

      ImGui::PushFont(&f);
      ImGui::Text("%s", m_deployOutput.c_str());
      ImGui::PopFont();

      ImGui::Separator();
      ImGui::Spacing();

      if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    // Handle any exceptions that are thrown from our threads.
    if (m_exception) {
      auto ex = m_exception;
      m_exception = std::exception_ptr();
      std::rethrow_exception(ex);
    }
  });
}

void Generator::DeployProject() {
  m_deployStatus = std::async(std::launch::async, [&] {
    try {
      m_deployOutput = "";
      m_creator.DeployProject(&m_deployOutput);
    } catch (const std::exception& e) {
      m_exception = std::current_exception();
    }
  });
}
