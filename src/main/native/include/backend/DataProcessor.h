// MIT License

#pragma once

#include <vector>

#include <units/acceleration.h>
#include <units/base.h>
#include <units/time.h>
#include <units/velocity.h>
#include <units/voltage.h>

namespace frcchar {
class DataProcessor {
 public:
  struct Data {
    units::volt_t voltage;
    units::meters_per_second_t velocity;
    units::meters_per_second_squared_t acceleration;
  };

  struct FeedforwardGains {
    units::volt_t Ks;
    decltype(1_V / 1_mps) Kv;
    decltype(1_V / 1_mps_sq) Ka;

    double rSquared;
  };

  struct FeedbackGains {
    double Kp;
    double Kd;
  };

  FeedforwardGains CalculateFeedforwardGains(const std::vector<Data>& data);

  FeedbackGains CalculatePositionFeedbackGains(const FeedforwardGains& ff,
                                               units::meter_t qp,
                                               units::meters_per_second_t qv,
                                               units::volt_t maxEffort,
                                               units::second_t period,
                                               units::second_t positionDelay);
  FeedbackGains CalculateVelocityFeedbackGains(const FeedforwardGains& ff,
                                               units::meters_per_second_t qv,
                                               units::volt_t maxEffort,
                                               units::second_t period,
                                               units::second_t sensorDelay);

 private:
  static constexpr units::meters_per_second_t kQuasistaticVelocityThreshold =
      0.1_fps;
};
}  // namespace frcchar
