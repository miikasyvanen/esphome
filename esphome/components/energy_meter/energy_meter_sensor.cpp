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
  this->high_freq_.start();
  // calcVI(this->zero_crossings_, this->sample_duration_);
  calcNewVI(this->zero_crossings_, this->sample_duration_);
  this->high_freq_.stop();

  this->current_->publish_state(Irms);
  this->voltage_->publish_state(Vrms);
  this->energy_consumed_->publish_state(energyConsumed);

  this->real_power_->publish_state(realPower);
  this->apparent_power_->publish_state(apparentPower);
  this->reactive_power_->publish_state(reactivePower);
  this->phase_angle_->publish_state(phaseAngle);
  this->power_factor_->publish_state(powerFactor);

  this->is_sampling_ = false;
}

void EnergyMeterSensor::loop() {
  if (!this->is_sampling_)
    return;
}

void EnergyMeterSensor::calcVI(unsigned int crossings, unsigned int timeout) {
  int SupplyVoltage = supply_voltage_;

  reactivePower = 0.0;
  phaseAngle = 0.0;

  unsigned int crossCount = 0;       // Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples = 0;  // This is now incremented

  double sampleVmin = 3.3;
  double sampleVmax = 0.0;
  double sampleImin = 3.3;
  double sampleImax = 0.0;

  unsigned int sampleVstart = 0.0;
  unsigned int sampleVend = 0.0;
  unsigned int sampleVinterval = 0.0;
  unsigned int sampleIstart = 0.0;
  unsigned int sampleIend = 0.0;
  unsigned int sampleIinterval = 0.0;

  unsigned int ac_pos_limit = ADC_COUNTS * 0.55;
  unsigned int ac_neg_limit = ADC_COUNTS * 0.45;

  unsigned long start = millis();

  ESP_LOGD(TAG, "Wait for waveform zero crossing...");
  int lastStartV = ADC_COUNTS;
  // Wait for the waveform to cross the 'zero' (offset) value while going positive side
  while (1) {
    float measVoltage = this->voltage_source_->sample();
    startV = measVoltage;  // * ADC_COUNTS / (SupplyVoltage / 1000.0);
    // Check if waveform crosses zero point going up
    if ((startV > lastStartV) && (startV > offsetV))
      break;
    if ((millis() - start) > timeout)
      break;
    lastStartV = startV;
  }
  ESP_LOGD(TAG, "Waveform crossed, start sampling...");

  unsigned int sampleStart = 0;
  unsigned int interval = 1000000 / sampleFreq;

  start = millis();

  while ((crossCount < crossings) && ((millis() - start) < timeout)) {
    sampleStart = micros();

    numberOfSamples++;          // Count number of samples
    lastFilteredV = filteredV;  // Used for delay/phase compensation

    //-----------------------------------------------------------------------------
    // A) Read in voltage and current samples
    //-----------------------------------------------------------------------------
    // ESP_LOGD(TAG, "ADC count: %d", ADC_COUNTS);
    sampleVstart = micros();
    double measVoltage = this->voltage_source_->sample();  // Read in voltage signal
    sampleVend = micros();
    sampleVinterval += sampleVend - sampleVstart;
    sampleIstart = micros();
    double measCurrent = this->current_source_->sample();  // Read in current signal
    sampleIend = micros();
    sampleIinterval += sampleIend - sampleIstart;
    // ESP_LOGD(TAG, "Measured ADC values: voltage = %.3f | current = %.3f", volt, curr);
    sampleV = measVoltage * ADC_COUNTS / (SupplyVoltage / 1000.0);  // Convert voltage sample to raw ADC value (0-4095)
    sampleI = measCurrent * ADC_COUNTS / (SupplyVoltage / 1000.0);  // Convert current sample to raw ADC value (0-4095)
    // ESP_LOGD(TAG, "Raw ADC values: voltage = %.3f | current = %.3f", sampleV, sampleI);

    // Save min and max points of sample set (just used for logging for now)
    if (sampleVmin > measVoltage)
      sampleVmin = measVoltage;
    if (sampleVmax < measVoltage)
      sampleVmax = measVoltage;
    if (sampleImin > measCurrent)
      sampleImin = measCurrent;
    if (sampleImax < measCurrent)
      sampleImax = measCurrent;

    //-----------------------------------------------------------------------------
    // B) Apply digital low pass filters to extract the 2.5 V or 1.65 V dc offset,
    //     then subtract this - signal is now centred on 0 counts.
    //-----------------------------------------------------------------------------
    offsetV = offsetV + ((sampleV - offsetV) / 4096);  // ADC_COUNTS);
    filteredV = sampleV - offsetV;
    offsetI = offsetI + ((sampleI - offsetI) / 4096);  // ADC_COUNTS);
    filteredI = sampleI - offsetI;

    //-----------------------------------------------------------------------------
    // C) Root-mean-square method voltage
    //-----------------------------------------------------------------------------
    sqV = filteredV * filteredV;  // 1) square voltage values
    sumV += sqV;                  // 2) sum

    //-----------------------------------------------------------------------------
    // D) Root-mean-square method current
    //-----------------------------------------------------------------------------
    sqI = filteredI * filteredI;  // 1) square current values
    sumI += sqI;                  // 2) sum

    //-----------------------------------------------------------------------------
    // E) Phase calibration
    //-----------------------------------------------------------------------------
    phaseShiftedV = lastFilteredV + PHASECAL * (filteredV - lastFilteredV);

    //-----------------------------------------------------------------------------
    // F) Instantaneous power calc
    //-----------------------------------------------------------------------------
    instP = phaseShiftedV * filteredI;  // Instantaneous Power
    sumP += instP;                      // Sum

    //-----------------------------------------------------------------------------
    // G) Find the number of times the voltage has crossed the initial voltage
    //    - every 2 crosses we will have sampled 1 wavelength
    //    - so this method allows us to sample an integer number of half wavelengths which increases accuracy
    //-----------------------------------------------------------------------------
    lastVCross = checkVCross;
    if (sampleV > startV)
      checkVCross = true;
    else if (sampleV < (startV - 50))
      checkVCross = false;
    if (numberOfSamples == 1)
      lastVCross = checkVCross;

    if (lastVCross != checkVCross)
      crossCount++;

    // Wait for interval to start next sample
    while (micros() < (sampleStart + interval)) {
    }
  }
  unsigned long convTime = millis() - start;
  ESP_LOGD(TAG, "Sampling done -> %lu samples in %lu zero crossings in %lu ms, calculate values...", numberOfSamples,
           crossCount, convTime);
  ESP_LOGD(TAG, "ADC avg intervals | V = %lu | I = %lu", sampleVinterval / numberOfSamples,
           sampleIinterval / numberOfSamples);
  // ESP_LOGD(TAG, "Measured ADC min/max values: Vmin = %.3f | Vmax = %.3f | Imin = %.3f | Imax = %.3f", sampleVmin,
  // sampleVmax, sampleImin, sampleImax);

  //-------------------------------------------------------------------------------------------------------------------------
  // 3) Post loop calculations
  //-------------------------------------------------------------------------------------------------------------------------
  // Calculation of the root of the mean of the voltage and current squared (rms)
  // Calibration coefficients applied.

  double V_RATIO = VCAL * ((SupplyVoltage / 1000.0) / (ADC_COUNTS));
  Vrms = V_RATIO * sqrt(sumV / numberOfSamples);

  double I_RATIO = ICAL * ((SupplyVoltage / 1000.0) / (ADC_COUNTS));
  Irms = I_RATIO * sqrt(sumI / numberOfSamples);

  energyConsumed = Irms * Vrms;  // false math

  // Calculation power values
  realPower = V_RATIO * I_RATIO * sumP / numberOfSamples;
  apparentPower = Vrms * Irms;
  powerFactor = realPower / apparentPower;

  ESP_LOGD(TAG, "Vrms = %.3f | Irms = %.3f | realPower = %.3f | apparentPower = %.3f | powerFactor = %.3f", Vrms, Irms,
           realPower, apparentPower, powerFactor);
  // ESP_LOGD(TAG, "Vinterval = %lu | Iinterval = %lu", sampleVinterval / numberOfSamples, sampleIinterval /
  // numberOfSamples);

  // Power P (W)   | Reactive Power Q (var) | Apparent Power S (VA)  | Power Factor PF    | Kulma (°)

  ESP_LOGD(TAG, "Values calculated, looping...");

  // Reset accumulators
  sumV = 0;
  sumI = 0;
  sumP = 0;
}

void EnergyMeterSensor::calcNewVI(unsigned int crossings, unsigned int timeout) {
  int SupplyVoltage = supply_voltage_;
  energyMultiplier = mainsFreq / zero_crossings_;

  unsigned int crossCount = 0;       // Used to measure number of times threshold is crossed.
  unsigned int numberOfSamples = 0;  // This is now incremented

  double sampleVmin = 3.3;
  double sampleVmax = 0.0;
  double sampleImin = 3.3;
  double sampleImax = 0.0;

  unsigned int sampleVstart = 0.0;
  unsigned int sampleVend = 0.0;
  unsigned int sampleVinterval = 0.0;
  unsigned int sampleIstart = 0.0;
  unsigned int sampleIend = 0.0;
  unsigned int sampleIinterval = 0.0;

  unsigned long start = millis();

  unsigned int sampleStart = 0;
  unsigned int interval = 1000000 / sampleFreq;

  int phaseVstart = 0;
  int phaseIstart = 0;

  double lastMeasVoltage = 0;
  double lastMeasCurrent = 0;

  int phaseDiff = 0;
  unsigned int numberOfPhaseSamples = 0;

  bool phaseVchecked = false;
  bool phaseIchecked = false;

  int lastSampleV = 4095;
  int lastSampleI = 4095;

  ESP_LOGD(TAG, "Wait for waveform zero crossing...");
  int lastStartV = ADC_COUNTS;
  // Wait for the waveform to cross the 'zero' (offset) value while going positive side
  while (1) {
    double measVoltage = this->voltage_source_->sample();
    startV = measVoltage * ADC_COUNTS / (SupplyVoltage / 1000.0);
    // Check if waveform crosses zero point going to positive
    if ((startV > lastStartV) && (startV > offsetV))
      break;
    if ((millis() - start) > timeout) {
      ESP_LOGE(TAG, "Waveform zero crossing TIMEOUT!");
      return;
    }
    lastStartV = startV;
  }
  ESP_LOGD(TAG, "Waveform crossed, start sampling...");

  start = millis();

  while ((crossCount < crossings) && ((millis() - start) < timeout)) {
    sampleStart = micros();

    numberOfSamples++;  // Count number of samples
    // lastFilteredV = filteredV;  //Used for delay/phase compensation

    sampleVstart = micros();
    double measVoltage = this->voltage_source_->sample();  // Read in voltage signal
    sampleVend = micros();
    sampleIstart = micros();
    double measCurrent = this->current_source_->sample();  // Read in current signal
    sampleIend = micros();
    // ESP_LOGD(TAG, "Measured ADC values: voltage = %.3f | current = %.3f", volt, curr);
    sampleV = measVoltage * ADC_COUNTS / (SupplyVoltage / 1000.0f);  // Convert voltage sample to raw ADC value (0-4095)
    sampleI = measCurrent * ADC_COUNTS / (SupplyVoltage / 1000.0f);  // Convert current sample to raw ADC value (0-4095)
    // ESP_LOGD(TAG, "Raw ADC values: voltage = %.3f | current = %.3f", sampleV, sampleI);

    sampleVinterval += sampleVend - sampleVstart;
    sampleIinterval += sampleIend - sampleIstart;

    /*******************
     *
     ******************/
    if ((sampleV > lastSampleV) && (sampleV > offsetV) && (!phaseVchecked)) {
      phaseVchecked = true;
      phaseVstart = micros() - sampleVstart;
      if (phaseIchecked)  // Check if current leading
      {
        phaseDiff += (phaseVstart - phaseIstart);
        numberOfPhaseSamples++;
      }
    }
    if ((sampleI > lastSampleI) && (sampleI > offsetI) && (!phaseIchecked)) {
      phaseIchecked = true;
      phaseIstart = micros() - sampleIstart;
      if (phaseVchecked)  // Check if current lagging
      {
        phaseDiff += (phaseVstart - phaseIstart);
        numberOfPhaseSamples++;
      }
    }
    if ((sampleV < lastSampleV) && (sampleV < (offsetV - 10))) {
      phaseDiff = 0;
      phaseVchecked = false;
      phaseIchecked = false;
      phaseVstart = 0;
      phaseIstart = 0;
    }

    lastSampleV = sampleV;
    lastSampleI = sampleI;
    /*******************
     *
     ******************/

    // Save min and max points of sample set (just used for logging for now)
    if (sampleVmin > measVoltage)
      sampleVmin = measVoltage;
    if (sampleVmax < measVoltage)
      sampleVmax = measVoltage;
    if (sampleImin > measCurrent)
      sampleImin = measCurrent;
    if (sampleImax < measCurrent)
      sampleImax = measCurrent;

    // offsetV = offsetV + ((sampleV - offsetV) / ADC_COUNTS);
    filteredV = sampleV - offsetV;
    // offsetI = offsetI + ((sampleI - offsetI) / ADC_COUNTS);
    filteredI = sampleI - offsetI;

    // phaseShiftedV = lastFilteredV + PHASECAL * (filteredV - lastFilteredV);

    // double V_RATIO = VCAL * ((SupplyVoltage/1000.0) / (ADC_COUNTS));
    // double I_RATIO = ICAL * ((SupplyVoltage/1000.0) / (ADC_COUNTS));
    // double newV = V_RATIO * (filteredV / ADC_COUNTS * (SupplyVoltage / 1000.0)) * VCAL;
    // double newI = I_RATIO * (filteredI / ADC_COUNTS * (SupplyVoltage / 1000.0)) * ICAL;
    double newV = filteredV / ADC_COUNTS * (SupplyVoltage / 1000.0) * VCAL;
    double newI = filteredI / ADC_COUNTS * (SupplyVoltage / 1000.0) * ICAL;
    updateVI(newI, newV);

    lastVCross = checkVCross;
    if (sampleV > offsetV)
      checkVCross = true;
    else if (sampleV < (offsetV - 10))
      checkVCross = false;
    if (numberOfSamples == 1)
      lastVCross = checkVCross;

    if (!lastVCross && checkVCross)
      crossCount++;

    // Wait for interval to start next sample
    while (micros() < (sampleStart + interval)) {
    }
  }
  unsigned long convTime = millis() - start;
  ESP_LOGD(TAG, "Sampling done -> %lu samples in %lu zero crossings in %lu ms, calculate values...", numberOfSamples,
           crossCount, convTime);
  ESP_LOGD(TAG, "ADC avg intervals | V = %lu | I = %lu", sampleVinterval / numberOfSamples,
           sampleIinterval / numberOfSamples);
  ESP_LOGD(TAG, "Measured ADC min/max values: Vmin = %.3f | Vmax = %.3f | Imin = %.3f | Imax = %.3f", sampleVmin,
           sampleVmax, sampleImin, sampleImax);
  ESP_LOGD(TAG, "Scaled ADC min/max values: Vmin = %.3f | Vmax = %.3f | Imin = %.3f | Imax = %.3f",
           (sampleVmin - 1.65) * VCAL, (sampleVmax - 1.65) * VCAL, (sampleImin - 1.65) * ICAL,
           (sampleImax - 1.65) * ICAL);
  ESP_LOGD(TAG, "ADC_BITS: %lu | ADC_COUNTS: %lu | offsetV: %.0f | offsetI: %.0f", ADC_BITS, ADC_COUNTS, offsetV,
           offsetI);
  // ESP_LOGD(TAG, "Phase Diff Samples: %lu, Phase Diff in us %d", numberOfPhaseSamples, (phaseDiff /
  // numberOfPhaseSamples));

  Irms = getI_RMS();
  Vrms = getV_RMS();
  if (Irms_ != 0)
    Irms = (Irms_ + Irms) / 2;
  if (Vrms_ != 0)
    Vrms = (Vrms_ + Vrms) / 2;

  Irms_ = Irms;
  Vrms_ = Vrms;

  // Real power is average of instantenous power over samples (U*I/numberOfSamples)
  realPower = getRealPower();
  // Apparent power (S) is the total power delivered to an AC circuit, calculated as the product of RMS voltage (Vrms)
  // and RMS current (Irms) without accounting for phase angle.
  apparentPower = getApparentPower();
  if (apparentPower_ != 0)
    apparentPower = (apparentPower_ + apparentPower) / 2;
  apparentPower_ = apparentPower;

  reactivePower = getReactivePower();
  powerFactor = getPowerFactor();
  phaseAngle = getPhaseAngle();
  // energyConsumed = getEnergyWh();
  energyConsumed = getInstantEnergyWh();

  ESP_LOGD(TAG,
           "Vrms = %.3f | Irms = %.3f | power = %.3f | realPower = %.3f | apparentPower = %.3f | powerFactor = %.3f",
           Vrms, Irms, apparentPower * 0.81, realPower, apparentPower, powerFactor);
  ESP_LOGD(TAG,
           "energyConsumed kWh = %.6f | energyConsumedInstant kWh = %.6f | energyConsumed Wh = %.6f | "
           "energyConsumedInstant Wh = %.6f",
           getEnergykWh(), getInstantEnergykWh(), getEnergyWh(), getInstantEnergyWh());
  ESP_LOGD(TAG, "Energy / h = %.6f kWh | Energy / d = %.6f kWh", getInstantEnergykWh() * 3600,
           getInstantEnergykWh() * 3600 * 24);

  // Power P (W)   | Reactive Power Q (var) | Apparent Power S (VA)  | Power Factor PF    | Kulma (°)

  ESP_LOGD(TAG, "Values calculated, looping...");

  cumulativeJouleInstant = 0;
  // V_buffer.clear();
  // I_buffer.clear();
  // sumIV = 0;
  // sumIV_delay = 0;
  // sumI2 = 0;
  // sumV2 = 0;
}

}  // namespace energy_meter
}  // namespace esphome
