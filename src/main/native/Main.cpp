#include <iostream>

#include <frc/geometry/Translation2d.h>
#include <frc/trajectory/TrajectoryGenerator.h>

#include <imgui.h>
#include <wpigui.h>

namespace gui = wpi::gui;

int main() {
  gui::CreateContext();
  gui::Initialize("FRC Characterization", 1280, 720);
  gui::Main();
  gui::DestroyContext();
}
