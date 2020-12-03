// MIT License

#include "backend/DataProcessor.h"

#include <Eigen/Core>
#include <frc/controller/LinearQuadraticRegulator.h>
#include <frc/system/plant/LinearSystemId.h>
#include <wpi/json.h>
#include <wpi/raw_istream.h>
#include <wpi/raw_ostream.h>

using namespace frcchar;

DataProcessor::DataProcessor(std::string* path, FFGains* ffGains,
                             FBGains* fbGains, GainPreset* preset,
                             LQRParameters* params)
    : m_path(*path),
      m_ffGains(*ffGains),
      m_fbGains(*fbGains),
      m_preset(*preset),
      m_lqrParams(*params) {
  // Parse the JSON and extract all the relevant data from it.
  std::error_code ec;
  wpi::raw_fd_istream input(m_path, ec);
  wpi::json json;
  input >> json;

  // Get the project type and units per rotation (factor).
  m_projectType = json.at("test").get<std::string>();
  m_factor = units::meter_t(json.at("unitsPerRotation").get<double>());

  // Extract the 4 primary data sets.
  RawData sf = json.at("slow-forward").get<RawData>();
  RawData sb = json.at("slow-backward").get<RawData>();
  RawData ff = json.at("fast-forward").get<RawData>();
  RawData fb = json.at("fast-backward").get<RawData>();

  // Clean the data to ensure that voltages have the correct signs and that all
  // conversion factors are applied.
  auto clean = [&](std::initializer_list<RawData*> vecs) {
    for (auto ptr : vecs) {
      for (auto&& pt : *ptr) {
        pt[3] = std::copysign(pt[3], pt[7]);
        pt[4] = std::copysign(pt[4], pt[8]);
        pt[7] *= m_factor.to<double>();
        pt[8] *= m_factor.to<double>();
      }
    }
  };

  clean({&sf, &sb, &ff, &fb});

  // Trim the quasistatic test data.
  TrimQuasistaticData(&sf);
  TrimQuasistaticData(&sb);

  // Get prepared quasistatic and step voltage data.
  auto sfp = PrepareDataForAnalysis(&sf);
  auto sbp = PrepareDataForAnalysis(&sb);
  auto ffp = PrepareDataForAnalysis(&ff);
  auto fbp = PrepareDataForAnalysis(&fb);

  // Trim prepared step-voltage data.
  TrimStepVoltageData(&ffp);
  TrimStepVoltageData(&fbp);

  // Store the data inside our map.
  auto& fwdEntry = m_data["Forward"];
  fwdEntry = std::move(sfp);
  fwdEntry.insert(fwdEntry.end(), std::make_move_iterator(ffp.begin()),
                  std::make_move_iterator(ffp.end()));
  auto& bwdEntry = m_data["Backward"];
  bwdEntry = std::move(sbp);
  bwdEntry.insert(bwdEntry.end(), std::make_move_iterator(fbp.begin()),
                  std::make_move_iterator(fbp.end()));
}

void DataProcessor::Update() {
  CalculateFeedforwardGains();
  if (m_preset.velocity)
    CalculateVelocityFeedbackGains();
  else
    CalculatePositionFeedbackGains();
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
std::vector<double> DataProcessor::PrepareDataForAnalysis(
    std::vector<std::array<double, 10>>* data) {
  // The format for the return vector is as follows:
  // voltage, intercept term, velocity, acceleration.

  // We will first create the return vector and pre-allocate the memory that
  // we require.
  std::vector<double> r;
  r.reserve(data->size() * 4 - 8);

  for (size_t i = 0; i < data->size(); ++i) {
    // We don't want to include the first and last data points because they
    // will purely be used for acceleration calculations.
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

void DataProcessor::TrimStepVoltageData(std::vector<double>* data) {
  // We want to find when the acceleration data roughly stops increasing at
  // the beginning.
  size_t idx = 3;

  // We will use this to make sure that the acceleration is decreasing for 3
  // consecutive entries in a row. This will help avoid false positives from
  // bad data.
  bool caution = false;

  // Iterate through the acceleration values and check where we hit the max.
  for (size_t i = 3; i < data->size(); i += 4) {
    // Get the current acceleration.
    double acceleration = std::abs(data->at(i));

    // If we are not in caution, the acceleration values are still
    // increasing..
    if (!caution) {
      if (acceleration < std::abs(data->at(idx)))
        // We found a potential candidate. Let's mark the flag and continue
        // checking...
        caution = true;
      else
        // Set the current acceleration to be the highest so far.
        idx = i;
    } else {
      // Check to make sure the acceleration value is still smaller. If it
      // isn't, break out of caution.
      if (acceleration >= std::abs(data->at(idx))) {
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

void DataProcessor::CalculateFeedforwardGains() {
  const std::vector<double>& vec = m_data[m_dataset];
  // The linear model can be written as follows:
  // y = Xβ + u, where y is the dependent observed variable, X is the matrix
  // of independent variables, β is a vector of coefficients, and u is a
  // vector of residuals.

  // We want to minimize u^2 = u'u = (y - Xβ)'(y - Xβ).
  // β = (X'X)^-1 (X'y)

  // Get the number of elements.
  int n = vec.size() / 4;

  Eigen::Map<const Eigen::MatrixXd, 0, Eigen::Stride<1, 4>> y(vec.data(), n, 1);
  Eigen::Map<const Eigen::MatrixXd, 0, Eigen::Stride<1, 4>> X(vec.data() + 1, n,
                                                              3);

  // Calculate b = β that minimizes u'u.
  Eigen::MatrixXd b = (X.transpose() * X).llt().solve(X.transpose() * y);

  // We will now calculate r^2 or the coefficient of determination, which
  // tells us how much of the total variation (variation in y) can be
  // explained by the regression model.

  // We will first calculate the sum of the squares of the error, or the
  // variation in error (SSE).
  double SSE = (y - X * b).squaredNorm();

  // Now we will calculate the total variation in y, known as SSTO.
  double SSTO = ((y.transpose() * y) - (1 / n) * (y.transpose() * y)).value();

  double rSquared = (SSTO - SSE) / SSTO;
  double adjRSquared = 1 - (1 - rSquared) * ((n - 1.0) / (n - 3));

  m_ffGains = {units::volt_t(b(0)), units::Kv_t(b(1)), units::Ka_t(b(2)),
               adjRSquared};
}

void DataProcessor::CalculatePositionFeedbackGains() {
  if (m_ffGains.Ka.to<double>() > 1E-7) {
    // Get the position system given Kv and Ka.
    auto system = frc::LinearSystemId::IdentifyPositionSystem<units::meter>(
        m_ffGains.Kv, m_ffGains.Ka);
    // Create LQR controller.
    auto lqr = frc::LinearQuadraticRegulator<2, 1>(
        system, {m_lqrParams.qp.to<double>(), m_lqrParams.qv.to<double>()},
        {m_lqrParams.maxEffort.to<double>()}, m_preset.dt);
    lqr.LatencyCompensate(system, m_preset.dt, m_preset.latency);

    m_fbGains = {lqr.K(0, 0), lqr.K(0, 1)};
  } else {
    auto system = frc::LinearSystem<1, 1, 1>(
        frc::MakeMatrix<1, 1>(0.0), frc::MakeMatrix<1, 1>(1.0),
        frc::MakeMatrix<1, 1>(1.0), frc::MakeMatrix<1, 1>(0.0));
    auto lqr = frc::LinearQuadraticRegulator<1, 1>(
        system, {m_lqrParams.qp.to<double>()}, {m_lqrParams.qv.to<double>()},
        m_preset.dt);
    lqr.LatencyCompensate(system, m_preset.dt, m_preset.latency);
    m_fbGains = {m_ffGains.Kv.to<double>() * lqr.K(0, 0), 0};
  }
}

void DataProcessor::CalculateVelocityFeedbackGains() {
  using namespace frc;

  // If acceleration for velocity control requires no effort, the feedback
  // control gains approach zero. We special-case it here because numerical
  // instabilities arise in the LQR otherwise.
  if (m_ffGains.Ka.to<double>() < 1E-7) m_fbGains = {0, 0};

  // Create a velocity system from our feedforward gains.
  auto sys = LinearSystemId::IdentifyVelocitySystem<units::meter>(m_ffGains.Kv,
                                                                  m_ffGains.Ka);
  // Create LQR controller for optimal gains.
  auto lqr = LinearQuadraticRegulator<1, 1>(
      sys, {m_lqrParams.qv.to<double>()}, {m_lqrParams.maxEffort.to<double>()},
      m_preset.dt);

  // Compensate for sensor delay.
  lqr.LatencyCompensate(sys, m_preset.dt, m_preset.latency);

  // Return gains.
  m_fbGains = {lqr.K(0, 0), 0};
}
