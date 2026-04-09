#pragma once

#include <vector>
#include <deque>
#include <cmath>

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

  void set_energy_consumed(sensor::Sensor *energy_consumed) { energy_consumed_ = energy_consumed; }
  void set_real_power(sensor::Sensor *real_power) { real_power_ = real_power; }
  void set_apparent_power(sensor::Sensor *apparent_power) { apparent_power_ = apparent_power; }
  void set_reactive_power(sensor::Sensor *reactive_power) { reactive_power_ = reactive_power; }
  void set_power_factor(sensor::Sensor *power_factor) { power_factor_ = power_factor; }
  void set_phase_angle(sensor::Sensor *phase_angle) { phase_angle_ = phase_angle; }

  void set_supply_voltage(float supply_voltage) { supply_voltage_ = supply_voltage; }
  void set_offset_voltage(float offset_voltage) { offset_voltage_ = offset_voltage; }
  void set_voltage_cal(float voltage_cal) { voltage_cal_ = voltage_cal; }
  void set_phase_cal(float phase_cal) { phase_cal_ = phase_cal; }

  void set_energy_consumed_value(double value) { energyConsumed = value; }

  void set_zero_crossings(uint8_t zero_crossings) { zero_crossings_ = zero_crossings; }

  void calcVI(unsigned int crossings, unsigned int timeout);
  double calcIrms(unsigned int Number_of_Samples);

  double realPower, apparentPower, reactivePower, phaseAngle, powerFactor, energyConsumed, Vrms, Irms;

 protected:
#define ADC_BITS 12
#define ADC_COUNTS (1 << ADC_BITS)

  /// High Frequency loop() requester used during sampling phase.
  HighFrequencyLoopRequester high_freq_;

  /// Duration in ms of the sampling phase.
  uint32_t sample_duration_;
  /// The sampling source to read values from.
  voltage_sampler::VoltageSampler *current_source_;
  voltage_sampler::VoltageSampler *voltage_source_;

  sensor::Sensor *voltage_{nullptr};
  sensor::Sensor *current_{nullptr};
  sensor::Sensor *energy_consumed_{nullptr};
  sensor::Sensor *real_power_{nullptr};
  sensor::Sensor *apparent_power_{nullptr};
  sensor::Sensor *reactive_power_{nullptr};
  sensor::Sensor *power_factor_{nullptr};
  sensor::Sensor *phase_angle_{nullptr};

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
  // Calibration coefficients
  // These need to be set in order to obtain accurate results
  double VCAL = 788.0;  // 236.26f;
  double ICAL = 10;
  double PHASECAL = 1.28;

  int startV;   // Instantaneous voltage at start of sample window.
  int sampleV;  // sample_ holds the raw analog read value
  int sampleI;

  double lastFilteredV, filteredV;  // Filtered_ is the raw analog value minus the DC offset
  double filteredI;
  double offsetV = ADC_COUNTS >> 1;  // Low-pass filter output
  double offsetI = ADC_COUNTS >> 1;  // Low-pass filter output

  double phaseShiftedV;  // Holds the calibrated phase shifted voltage.

  double sqV, sumV, sqI, sumI, instP, sumP;  // sq = squared, sum = Sum, inst = instantaneous

  bool lastVCross, checkVCross;  // Used to measure number of times threshold is crossed.

  double Vrms_;
  double Irms_;
  double apparentPower_;

  double supply_voltage_ = 3300.0;
  double offset_voltage_ = 1.65;
  double voltage_cal_ = 236.26;
  double phase_cal_ = 1.7;
  uint8_t zero_crossings_ = 0;
  bool is_sampling_ = false;

  double current_last_value_ = 0.0;
  double current_sample_sum_ = 0.0;
  double current_sample_squared_sum_ = 0.0;
  uint32_t current_num_samples_ = 0;
  bool current_is_sampling_ = false;

  double voltage_last_value_ = 0.0;
  double voltage_sample_sum_ = 0.0;
  double voltage_sample_squared_sum_ = 0.0;
  uint32_t voltage_num_samples_ = 0;
  bool voltage_is_sampling_ = false;

  void calcNewVI(unsigned int crossings, unsigned int timeout);

  unsigned int sampleVbuffer[320];

  const double mainsFreq = 50.0;
  const double sampleFreq = 800.0;
  const double Y = 1 / sin(2 * M_PI * mainsFreq / sampleFreq);
  const double DT = 1.0 / sampleFreq;
  double energyMultiplier = 0.0;

  size_t windowSize = 80;
  std::deque<double> I_buffer, V_buffer;

  double sumIV = 0.0, sumIV_delay = 0.0;
  double sumI2 = 0.0, sumV2 = 0.0;
  double cumulativeJoule = 0.0;
  double cumulativeJouleInstant = 0.0;

  void updateVI(double newI, double newV) {
    double lastV = V_buffer.empty() ? 0.0 : V_buffer.back();

    sumIV += newI * newV;
    sumIV_delay += newI * lastV;
    sumI2 += newI * newI;
    sumV2 += newV * newV;

    I_buffer.push_back(newI);
    V_buffer.push_back(newV);

    if (I_buffer.size() > windowSize) {
      double oldI = I_buffer.front();
      double oldV = V_buffer.front();
      I_buffer.pop_front();
      V_buffer.pop_front();
      sumIV -= oldI * oldV;
      sumIV_delay -= I_buffer.front() * oldV;
      sumI2 -= oldI * oldI;
      sumV2 -= oldV * oldV;
    }
    if (newI * newV > 0) {
      cumulativeJoule += (newI * newV) * DT * energyMultiplier;
      cumulativeJouleInstant += (newI * newV) * DT * energyMultiplier;
    }
  }

  double getRealPower() const { return (I_buffer.empty()) ? 0 : sumIV / I_buffer.size(); }
  double getReactivePower() const { return (I_buffer.empty()) ? 0 : (Y * sumIV_delay) / I_buffer.size(); }
  double getI_RMS() const { return std::sqrt(sumI2 / I_buffer.size()); }
  double getV_RMS() const { return std::sqrt(sumV2 / V_buffer.size()); }
  double getApparentPower() const { return getI_RMS() * getV_RMS(); }
  double getPhaseAngle() const { return std::atan2(Y * sumIV_delay, sumIV) * (180.0 / M_PI); }
  double getEnergykWh() const { return cumulativeJoule / 3600000.0; }
  double getInstantEnergykWh() const { return cumulativeJouleInstant / 3600000.0; }
  double getEnergyWh() const { return cumulativeJoule / 3600.0; }
  double getInstantEnergyWh() const { return cumulativeJouleInstant / 3600.0; }
  double getPowerFactor() const {
    double s = getApparentPower();
    // return s * (cos (getPhaseAngle() * (M_PI / 180.0)));
    return (s < 0.001) ? 0 : std::abs(getRealPower() / s);
  }
};

}  // namespace energy_meter
}  // namespace esphome
