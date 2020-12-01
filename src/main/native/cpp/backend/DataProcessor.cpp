// MIT License

#include "backend/DataProcessor.h"

#include <Eigen/Core>
#include <Eigen/SVD>
#include <frc/controller/LinearQuadraticRegulator.h>
#include <frc/system/plant/LinearSystemId.h>
#include <wpi/json.h>
#include <wpi/raw_ostream.h>
#include <wpi/raw_istream.h>

using namespace frcchar;

std::tuple<DataProcessor::FFGains, DataProcessor::FBGains>
DataProcessor::CalculateParameters(const std::string& path) {
  // Parse the JSON file.
  std::error_code ec;
  wpi::raw_fd_istream in(path, ec);
  wpi::json json;
  in >> json;

  // Get the raw data from all four runs.
  auto sf = json.at("slow-forward").get<std::vector<std::array<double, 10>>>();
  auto sb = json.at("slow-backward").get<std::vector<std::array<double, 10>>>();
  auto ff = json.at("fast-forward").get<std::vector<std::array<double, 10>>>();
  auto fb = json.at("fast-backward").get<std::vector<std::array<double, 10>>>();

  // Clean up the raw data a little bit.
  RawData* rd[] = {&sf, &sb, &ff, &fb};
  for (RawData* r : rd) {
    for (auto&& pt : *r) {
      // Ensure voltage sign matches velocity sign and convert rotation
      // measurements into proper units.
      pt[3] = std::copysign(pt[3], pt[7]);
      pt[7] *= m_settings.unitsPerRotation.to<double>();
    }
  }

  // Trim the quasistatic test data.
  TrimQuasistaticData(&sf);
  TrimQuasistaticData(&sb);

  // Get prepared quasistatic and step voltage data.
  auto sfp = ComputeAcceleration(&sf);
  auto sbp = ComputeAcceleration(&sb);
  auto ffp = ComputeAcceleration(&ff);
  auto fbp = ComputeAcceleration(&fb);

  // Trim prepared step-voltage data.
  TrimStepVoltageData(&ffp);
  TrimStepVoltageData(&fbp);

  // Pool all of the data together.
  std::vector<double> p;
  p.insert(p.end(), std::make_move_iterator(sfp.begin()),
           std::make_move_iterator(sfp.end()));
  p.insert(p.end(), std::make_move_iterator(sbp.begin()),
           std::make_move_iterator(sbp.end()));
  p.insert(p.end(), std::make_move_iterator(ffp.begin()),
           std::make_move_iterator(ffp.end()));
  p.insert(p.end(), std::make_move_iterator(fbp.begin()),
           std::make_move_iterator(fbp.end()));

  // Get feedforward gains from the pooled data.
  auto ffGains = CalculateFeedforwardGains(p);
  return {ffGains, FBGains{0.0, 0.0}};
}

std::vector<double> DataProcessor::PrepareData(
    std::vector<std::array<double, 10>>* data) {
  // Create a vector with the cleaned up data and pre-allocate all the memory
  // that we need.
  std::vector<double> r;
  r.reserve(data->size() * 4);

  // We will first do a pass cleaning up the data.
  auto& v = *data;
  for (auto&& pt : v) {
    // Ensure voltage sign matches velocity sign and convert rotation
    // measurements into proper units.
    pt[3] = std::copysign(pt[3], pt[7]);
    pt[7] *= m_settings.unitsPerRotation.to<double>();
  }

  // Iterate through the data and add samples that pass all our filters.
  for (size_t i = 0; i < v.size(); ++i) {
    // We can throw out the first and last data points because we won't have
    // acceleration values.
    if (i == 0 || i == v.size() - 1) continue;

    // If the absolute value of the velocity is less than the minimum threshold,
    // we can throw it out.
    if (std::abs(v[i][7]) < kQuasistaticVelocityThreshold.to<double>())
      continue;

    // Calculate acceleration.
    double acceleration =
        (v[i + 1][7] - v[i - 1][7]) / (v[i + 1][0] - v[i - 1][0]);

    r.push_back(v[i][3]);
    r.push_back(std::copysign(1.0, v[i][7]));
    r.push_back(v[i][7]);
    r.push_back(acceleration);
  }
  return r;
}

std::pair<std::vector<double>, std::vector<double>>
DataProcessor::PrepareDrivetrainData(
    const std::vector<std::array<double, 10>>& data) {
  // Create vectors with the cleaned up data and pre-allocate all the memory
  // that we need.
  std::vector<double> l;
  std::vector<double> r;
  l.reserve(data.size() * 4);
  r.reserve(data.size() * 4);

  // Iterate through the data and add samples that pass all our filters.
  for (size_t i = 0; i < data.size(); ++i) {
    // We can throw out the first and last data points because we won't have
    // acceleration values.
    if (i == 0 || i == data.size() - 1) continue;

    // If the absolute value of the velocity is less than the minimum threshold,
    // we can throw it out.
    if (std::abs(data[i][7]) < kQuasistaticVelocityThreshold.to<double>() ||
        std::abs(data[i][8]) < kQuasistaticVelocityThreshold.to<double>())
      continue;

    // Calculate accelerations.
    double lAcceleration = (std::abs(data[i + 1][7] - data[i - 1][7])) /
                           (data[i + 1][0] - data[i - 1][0]);
    double rAcceleration = (std::abs(data[i + 1][8] - data[i - 1][8])) /
                           (data[i + 1][0] - data[i - 1][0]);

    l.push_back(std::copysign(data[i][3], data[i][7]));
    l.push_back(std::copysign(1, data[i][7]));
    l.push_back(data[i][7]);
    l.push_back(lAcceleration);

    l.push_back(std::copysign(data[i][4], data[i][8]));
    l.push_back(std::copysign(1.0, data[i][8]));
    l.push_back(data[i][8]);
    l.push_back(rAcceleration);
  }

  return {l, r};
}

DataProcessor::FFGains DataProcessor::CalculateFeedforwardGains(
    const std::vector<double>& vec) {
  // The linear model can be written as follows:
  // y = Xβ + u, where y is the dependent observed variable, X is the matrix of
  // independent variables, β is a vector of coefficients, and u is a vector of
  // residuals.

  // We want to minimize u^2 = u'u = (y - Xβ)'(y - Xβ).
  // β = (X'X)^-1 (X'y)

  // Get the number of elements.
  int n = vec.size() / 4;

  Eigen::Map<const Eigen::MatrixXd, 0, Eigen::Stride<1, 4>> y(vec.data(), n, 1);
  Eigen::Map<const Eigen::MatrixXd, 0, Eigen::Stride<1, 4>> X(vec.data() + 1, n,
                                                              3);

  // Print y and X for debugging.
  std::cout << y << std::endl;
  std::cout << X << std::endl;

  // Calculate b = β that minimizes u'u.
  Eigen::MatrixXd b = (X.transpose() * X).llt().solve(X.transpose() * y);

  // We will now calculate r^2 or the coefficient of determination, which tells
  // us how much of the total variation (variation in y) can be explained by the
  // regression model.

  // We will first calculate the sum of the squares of the error, or the
  // variation in error (SSE).
  double SSE = (y - X * b).squaredNorm();

  // Now we will calculate the total variation in y, known as SSTO.
  double SSTO = ((y.transpose() * y) - (1 / n) * (y.transpose() * y)).value();

  double rSquared = (SSTO - SSE) / SSTO;
  double adjRSquared = 1 - (1 - rSquared) * ((n - 1.0) / (n - 3));

  return {units::volt_t(b(0)), units::Kv_t(b(1)), units::Ka_t(b(2)),
          adjRSquared};
}

DataProcessor::FBGains DataProcessor::CalculatePositionFeedbackGains(
    const DataProcessor::FFGains& ff, units::meter_t qp,
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

    return DataProcessor::FBGains{lqr.K(0, 0), lqr.K(0, 1)};
  } else {
    auto system = frc::LinearSystem<1, 1, 1>(
        frc::MakeMatrix<1, 1>(0.0), frc::MakeMatrix<1, 1>(1.0),
        frc::MakeMatrix<1, 1>(1.0), frc::MakeMatrix<1, 1>(0.0));
    auto lqr = frc::LinearQuadraticRegulator<1, 1>(system, {qp.to<double>()},
                                                   {qv.to<double>()}, period);
    lqr.LatencyCompensate(system, period, positionDelay);
    return DataProcessor::FBGains{ff.Kv.to<double>() * lqr.K(0, 0), 0};
  }
}

DataProcessor::FBGains DataProcessor::CalculateVelocityFeedbackGains(
    const DataProcessor::FFGains& ff, units::meters_per_second_t qv,
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

void DataProcessor::TrimQuasistaticData(
    std::vector<std::array<double, 10>>* data) {
  data->erase(
      std::remove_if(data->begin(), data->end(),
                     [](const auto& pt) {
                       return std::abs(pt[3]) <= 0 || std::abs(pt[4]) <= 0 ||
                              std::abs(pt[7]) <=
                                  kQuasistaticVelocityThreshold.to<double>() ||
                              std::abs(pt[8]) <=
                                  kQuasistaticVelocityThreshold.to<double>();
                     }),
      data->end());
}

void DataProcessor::TrimStepVoltageData(std::vector<double>* data) {
  // We want to find when the acceleration data roughly stops increasing at the
  // beginning.
  size_t idx = 3;

  // We will use this to make sure that the acceleration is decreasing for 3
  // consecutive entries in a row. This will help avoid false positives from bad
  // data.
  bool caution = false;

  // Iterate through the acceleration values and check where we hit the max.
  for (size_t i = 3; i < data->size(); i += 4) {
    // Get the current acceleration.
    double acceleration = data->at(i);

    // If we are not in caution, the acceleration values are still increasing..
    if (!caution) {
      if (acceleration <= data->at(idx))
        // We found a potential candidate. Let's mark the flag and continue
        // checking...
        caution = true;
      else
        // Set the current acceleration to be the highest so far.
        idx = i;
    } else {
      // Check to make sure the acceleration value is still smaller. If it
      // isn't, break out of caution.
      if (acceleration > data->at(idx)) {
        caution = false;
        idx = i;
      }
    }

    // If we were in caution for three iterations, we can exit.
    if (caution && (i - idx) == 12) break;
  }

  wpi::outs() << "[INFO] Exit step voltage trim at " << idx << " out of "
              << data->size() << "\n";

  // Remove all values before that maximum.
  data->erase(data->begin(), data->begin() + idx - 3);

  // Make sure that we still have a multiple of 4 elements in the vector. This
  // is to catch off-by-one errors.
  assert(data->size() % 4 == 0);
}

std::vector<double> DataProcessor::ComputeAcceleration(
    std::vector<std::array<double, 10>>* data) {
  // The format for the return vector is as follows:
  // voltage, intercept term, velocity, acceleration.

  // We will first create the return vector and pre-allocate the memory that we
  // require.
  std::vector<double> r;
  r.reserve(data->size() * 4 - 8);

  for (size_t i = 0; i < data->size(); ++i) {
    // We don't want to include the first and last data points because they will
    // purely be used for acceleration calculations.
    if (i == 0 || i == data->size() - 1) continue;

    // Add voltage, intercept term, and velocity.
    r.push_back(data->at(i)[3]);
    r.push_back(std::copysign(1.0, data->at(i)[7]));
    r.push_back(data->at(i)[7]);

    // Calculate acceleration.
    double acceleration = (data->at(i + 1)[7] - data->at(i - 1)[7]) /
                          (data->at(i + 1)[0] - data->at(i - 1)[0]);

    // Add the acceleration value to the vector.
    r.push_back(acceleration);
  }

  return r;
}
