#include <iostream>

#include <frc/geometry/Translation2d.h>
#include <frc/trajectory/TrajectoryGenerator.h>

int main() {
  auto trajectory = frc::TrajectoryGenerator::GenerateTrajectory(
      frc::Pose2d(5_m, 2_m, 0_rad), {}, frc::Pose2d(7_m, 5_m, 90_deg),
      frc::TrajectoryConfig(2_mps, 2_mps_sq));

  for (auto state : trajectory.States()) {
    std::cout << state.pose.X() << ", " << state.pose.Y() << std::endl;
  }
}
