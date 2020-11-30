// MIT License

#include "backend/DataProcessor.h"

#include <Eigen/Core>
#include <Eigen/SVD>
#include <frc/controller/LinearQuadraticRegulator.h>
#include <frc/system/plant/LinearSystemId.h>
#include <wpi/raw_ostream.h>

using namespace frcchar;

DataProcessor::FeedforwardGains DataProcessor::CalculateFeedforwardGains(
    const std::vector<DataProcessor::Data>& data) {
  // The linear model can be written as follows:
  // y = Xβ + u, where y is the dependent observed variable, X is the matrix of
  // independent variables, β is a vector of coefficients, and u is a vector of
  // residuals.

  // We want to minimize u^2 = u'u = (y - Xβ)'(y - Xβ).
  // β = (X'X)^-1 (X'y)

  // Get the number of elements.
  int n = data.size();

  Eigen::MatrixXd y(n, 1);
  Eigen::MatrixXd X(n, 3);

  // Fill the y and X matrices.
  for (int i = 0; i < n; ++i) {
    // Get the data point at this index.
    const auto& pt = data.at(i);

    // Add voltage to y.
    y(i, 0) = pt.voltage.to<double>();

    // Add velocity and acceleration to x. We add 1 for the initial column to
    // calculate the intercept.
    X(i, 0) = pt.velocity.to<double>() > 0 ? 1 : -1;
    X(i, 1) = pt.velocity.to<double>();
    X(i, 2) = pt.acceleration.to<double>();
  }

  // Calculate b = β that minimizes u'u.
  Eigen::MatrixXd b = (X.transpose() * X).ldlt().solve(X.transpose() * y);

  // We will now calculate r^2 or the coefficient of determination, which tells
  // us how much of the total variation (variation in y) can be explained by the
  // regression model.

  // We will first calculate the sum of the squares of the error, or the
  // variation in error (SSE).
  double SSE = (y - X * b).squaredNorm();

  // Now we will calculate the total variation in y, known as SSTO.
  double SSTO =
      ((y.transpose() * y) -
       (1 / n) * (y.transpose() * Eigen::MatrixXd::Identity(n, n) * y))(0, 0);

  double rSquared = (SSTO - SSE) / SSTO;
  double adjRSquared = 1 - (1 - rSquared) * ((n - 1) / (n - 3));

  return FeedforwardGains{units::volt_t(b(0, 0)),
                          decltype(1_V / 1_mps)(b(1, 0)),
                          decltype(1_V / 1_mps_sq)(b(2, 0)), adjRSquared};
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
