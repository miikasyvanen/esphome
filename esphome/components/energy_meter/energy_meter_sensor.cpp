#include "energy_meter_sensor.h"

#include "esphome/core/log.h"
#include <cinttypes>
#include <cmath>

namespace esphome {
namespace energy_meter {

static const char *const TAG = "energy_meter";

void EnergyMeterSensor::dump_config() {
  LOG_SENSOR("", "Energy Meter Sensor", this);
  ESP_LOGCONFIG(TAG, "  Sample Duration: %.2fs", this->sample_duration_ / 1e3f);
  LOG_UPDATE_INTERVAL(this);
}

void EnergyMeterSensor::update() {
  // Update only starts the sampling phase, in loop() the actual sampling is happening.

  // Request a high loop() execution interval during sampling phase.
  this->high_freq_.start();

  // Set timeout for ending sampling phase
  this->set_timeout("read", this->sample_duration_, [this]() {
    this->current_is_sampling_ = false;
    this->voltage_is_sampling_ = false;
    this->high_freq_.stop();

    if (this->current_num_samples_ == 0) {
      // Shouldn't happen, but let's not crash if it does.
      this->publish_state(NAN);
      return;
    }
    if (this->voltage_num_samples_ == 0) {
      // Shouldn't happen, but let's not crash if it does.
      this->publish_state(NAN);
      return;
    }

    const float current_rms_ac_dc_squared = this->current_sample_squared_sum_ / this->current_num_samples_;
    const float current_rms_dc = this->current_sample_sum_ / this->current_num_samples_;
    const float current_rms_ac_squared = current_rms_ac_dc_squared - current_rms_dc * current_rms_dc;
    float current_rms_ac = 0;
    if (current_rms_ac_squared > 0)
      current_rms_ac = std::sqrt(current_rms_ac_squared);
    ESP_LOGD(TAG, "'%s (A)' - Raw AC Value: %.3fA after %" PRIu32 " different samples (%" PRIu32 " SPS)",
             this->name_.c_str(), current_rms_ac, this->current_num_samples_,
             1000 * this->current_num_samples_ / this->sample_duration_);
    this->publish_state(current_rms_ac);

    const float voltage_rms_ac_dc_squared = this->voltage_sample_squared_sum_ / this->voltage_num_samples_;
    const float voltage_rms_dc = this->voltage_sample_sum_ / this->voltage_num_samples_;
    const float voltage_rms_ac_squared = voltage_rms_ac_dc_squared - voltage_rms_dc * voltage_rms_dc;
    float voltage_rms_ac = 0;
    if (voltage_rms_ac_squared > 0)
      voltage_rms_ac = std::sqrt(voltage_rms_ac_squared);
    ESP_LOGD(TAG, "'%s (V)' - Raw AC Value: %.3fV after %" PRIu32 " different samples (%" PRIu32 " SPS)",
             this->name_.c_str(), voltage_rms_ac, this->voltage_num_samples_,
             1000 * this->voltage_num_samples_ / this->sample_duration_);
    this->publish_state(voltage_rms_ac);
  });

  // Set sampling values
  this->current_last_value_ = 0.0;
  this->current_num_samples_ = 0;
  this->current_sample_sum_ = 0.0f;
  this->current_sample_squared_sum_ = 0.0f;
  this->current_is_sampling_ = true;

  this->voltage_last_value_ = 0.0;
  this->voltage_num_samples_ = 0;
  this->voltage_sample_sum_ = 0.0f;
  this->voltage_sample_squared_sum_ = 0.0f;
  this->voltage_is_sampling_ = true;
}

void EnergyMeterSensor::loop() {
  if (!this->current_is_sampling_)
    return;
  if (!this->voltage_is_sampling_)
    return;

  // Perform a single sample
  float current_value = this->current_source_->sample();
  if (std::isnan(current_value))
    return;

  // Assuming a sine wave, avoid requesting values faster than the ADC can provide them
  if (this->current_last_value_ == current_value)
    return;
  this->current_last_value_ = current_value;

  this->current_num_samples_++;
  this->current_sample_sum_ += current_value;
  this->current_sample_squared_sum_ += current_value * current_value;

  // Perform a single sample
  float voltage_value = this->voltage_source_->sample();
  if (std::isnan(voltage_value))
    return;

  // Assuming a sine wave, avoid requesting values faster than the ADC can provide them
  if (this->voltage_last_value_ == voltage_value)
    return;
  this->voltage_last_value_ = voltage_value;

  this->voltage_num_samples_++;
  this->voltage_sample_sum_ += voltage_value;
  this->voltage_sample_squared_sum_ += voltage_value * voltage_value;
}

}  // namespace energy_meter
}  // namespace esphome
