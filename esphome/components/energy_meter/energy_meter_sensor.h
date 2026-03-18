#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"

namespace esphome {
namespace energy_meter {

class EnergyMeterSensor : public sensor::Sensor, public PollingComponent {
 public:
  void update() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override {
    // After the base sensor has been initialized
    return setup_priority::DATA - 1.0f;
  }

  void set_sample_duration(uint32_t sample_duration) { sample_duration_ = sample_duration; }
  // void set_source(voltage_sampler::VoltageSampler *source) { source_ = source; }

  void set_current(sensor::Sensor *current) { current_ = current; }
  void set_voltage(sensor::Sensor *voltage) { voltage_ = voltage; }
  void set_current_source(voltage_sampler::VoltageSampler *source) { current_source_ = source; }
  void set_voltage_source(voltage_sampler::VoltageSampler *source) { voltage_source_ = source; }

 protected:
  /// High Frequency loop() requester used during sampling phase.
  HighFrequencyLoopRequester high_freq_;

  /// Duration in ms of the sampling phase.
  uint32_t sample_duration_;
  /// The sampling source to read values from.
  voltage_sampler::VoltageSampler *current_source_;
  voltage_sampler::VoltageSampler *voltage_source_;

  sensor::Sensor *voltage_{nullptr};
  sensor::Sensor *current_{nullptr};

  /** The DC offset of the circuit.
   *
   * Diagram: https://learn.openenergymonitor.org/electricity-monitoring/ct-sensors/interface-with-arduino
   *
   * The current clamp only measures AC, so any DC component is an unwanted artifact from the
   * sampling circuit. The AC component is essentially the same as the calculating the Standard-Deviation,
   * which can be done by cumulating 3 values per sample:
   *   1) Number of samples
   *   2) Sum of samples
   *   3) Sum of sample squared
   * https://en.wikipedia.org/wiki/Root_mean_square
   */

  float current_last_value_ = 0.0f;
  float current_sample_sum_ = 0.0f;
  float current_sample_squared_sum_ = 0.0f;
  uint32_t current_num_samples_ = 0;
  bool current_is_sampling_ = false;

  float voltage_last_value_ = 0.0f;
  float voltage_sample_sum_ = 0.0f;
  float voltage_sample_squared_sum_ = 0.0f;
  uint32_t voltage_num_samples_ = 0;
  bool voltage_is_sampling_ = false;
};

}  // namespace energy_meter
}  // namespace esphome
