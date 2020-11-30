// MIT License

#include "backend/DataProcessor.h"

#include <Eigen/Core>
#include <Eigen/SVD>
#include <frc/controller/LinearQuadraticRegulator.h>
#include <frc/system/plant/LinearSystemId.h>

using namespace frcchar;

DataProcessor::FeedforwardGains DataProcessor::CalculateFeedforwardGains(
    const std::vector<DataProcessor::Data>& data) {
  // https://genome.sph.umich.edu/w/images/2/2c/Biostat615-lecture14-presentation.pdf
  int n = data.size();
  int p = 3;

  Eigen::MatrixXd y(n, 1);
  Eigen::MatrixXd X(n, p);

  for (int i = 0; i < n; ++i) {
    // Get the data point at this index.
    const auto& pt = data.at(i);

    // Add voltage to y.
    y(i, 0) = pt.voltage.to<double>();

    // Add velocity and acceleration to x. We add 1 for the initial column to
    // calculate the intercept.
    X(i, 0) = 1;
    X(i, 1) = pt.velocity.to<double>();
    X(i, 2) = pt.acceleration.to<double>();
  }

  Eigen::JacobiSVD<Eigen::MatrixXd> svd(
      X, Eigen::ComputeThinU | Eigen::ComputeThinV);
  Eigen::MatrixXd betasSvd = svd.solve(y);

  return FeedforwardGains{units::volt_t(betasSvd(0, 0)),
                          decltype(1_V / 1_mps)(betasSvd(1, 0)),
                          decltype(1_V / 1_mps_sq)(betasSvd(2, 0))};
}
DataProcessor::FeedbackGains DataProcessor::CalculatePositionFeedbackGains(
    const DataProcessor::FeedforwardGains& ff, units::meter_t qp,
    units::meters_per_second_t qv, units::volt_t maxEffort,
    units::second_t period, units::second_t positionDelay) {
  if (ff.Ka.to<double>() > 1E-7) {
    // Get the position system given Kv and Ka.
    auto system =
        frc::LinearSystemId::IdentifyPositionSystem<units::meter>(ff.Kv, ff.Ka);
    // Create LQR controller.
    auto lqr = frc::LinearQuadraticRegulator<2, 1>(
        system, {qp.to<double>(), qv.to<double>()}, {maxEffort.to<double>()},
        period);
    lqr.LatencyCompensate(system, period, positionDelay);

    return DataProcessor::FeedbackGains{lqr.K(0, 0), lqr.K(0, 1)};
  } else {
    auto system = frc::LinearSystem<1, 1, 1>(
        frc::MakeMatrix<1, 1>(0.0), frc::MakeMatrix<1, 1>(1.0),
        frc::MakeMatrix<1, 1>(1.0), frc::MakeMatrix<1, 1>(0.0));
    auto lqr = frc::LinearQuadraticRegulator<1, 1>(system, {qp.to<double>()},
                                                   {qv.to<double>()}, period);
    lqr.LatencyCompensate(system, period, positionDelay);
    return DataProcessor::FeedbackGains{ff.Kv.to<double>() * lqr.K(0, 0), 0};
  }
}

DataProcessor::FeedbackGains DataProcessor::CalculateVelocityFeedbackGains(
    const DataProcessor::FeedforwardGains& ff, units::meters_per_second_t qv,
    units::volt_t maxEffort, units::second_t period,
    units::second_t sensorDelay) {
  using namespace frc;

  // If acceleration for velocity control requires no effort, the feedback
  // control gains approach zero. We special-case it here because numerical
  // instabilities arise in the LQR otherwise.
  if (ff.Ka.to<double>() < 1E-7) return {0, 0};

  // Create a velocity system from our feedforward gains.
  auto sys = LinearSystemId::IdentifyVelocitySystem<units::meter>(ff.Kv, ff.Ka);
  // Create LQR controller for optimal gains.
  auto lqr = LinearQuadraticRegulator<1, 1>(sys, {qv.to<double>()},
                                            {maxEffort.to<double>()}, period);
  // Compensate for sensor delay.
  lqr.LatencyCompensate(sys, period, sensorDelay);

  // Return gains.
  return {lqr.K(0, 0), 0};
}
