import esphome.codegen as cg
from esphome.components import sensor, voltage_sampler
import esphome.config_validation as cv
from esphome.const import (
    CONF_APPARENT_POWER,
    CONF_CURRENT,
    CONF_ENERGY_CONSUMED,
    CONF_ID,
    CONF_PHASE_ANGLE,
    CONF_POWER,
    CONF_POWER_FACTOR,
    CONF_REACTIVE_POWER,
    CONF_SENSOR_CURRENT,
    CONF_SENSOR_VOLTAGE,
    CONF_VOLTAGE,
    CONF_ZERO_CROSSINGS,
    DEVICE_CLASS_APPARENT_POWER,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_POWER_FACTOR,
    DEVICE_CLASS_REACTIVE_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_DEGREES,
    UNIT_EMPTY,
    UNIT_VOLT,
    UNIT_VOLT_AMPS,
    UNIT_VOLT_AMPS_REACTIVE,
    UNIT_WATT,
    UNIT_WATT_HOURS,
)

AUTO_LOAD = ["voltage_sampler"]
CODEOWNERS = ["@miikasyvanen"]

CONF_SAMPLE_DURATION = "sample_duration"

energy_meter_ns = cg.esphome_ns.namespace("energy_meter")
EnergyMeterSensor = energy_meter_ns.class_("EnergyMeterSensor", cg.PollingComponent)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(EnergyMeterSensor),
            cv.Required(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Required(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ENERGY_CONSUMED): sensor.sensor_schema(
                # EnergyMeterSensor,
                unit_of_measurement=UNIT_WATT_HOURS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_APPARENT_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT_AMPS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_APPARENT_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REACTIVE_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT_AMPS_REACTIVE,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_REACTIVE_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER_FACTOR): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_POWER_FACTOR,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PHASE_ANGLE): sensor.sensor_schema(
                unit_of_measurement=UNIT_DEGREES,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_EMPTY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            # ).extend(
            #    {
            cv.Required(CONF_SENSOR_CURRENT): cv.use_id(voltage_sampler.VoltageSampler),
            cv.Required(CONF_SENSOR_VOLTAGE): cv.use_id(voltage_sampler.VoltageSampler),
            cv.Optional(CONF_ZERO_CROSSINGS, default=20): cv.int_range(min=1, max=255),
            cv.Optional(
                CONF_SAMPLE_DURATION, default="200ms"
            ): cv.positive_time_period_milliseconds,
            #    }
        }
    )
    # .extend(
    #    {
    #        cv.GenerateID(CONF_ID): cv.use_id(EnergyMeterSensor),
    #    }
    # )
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    # var = await sensor.new_sensor(config)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if current_config := config.get(CONF_CURRENT):
        sens = await sensor.new_sensor(current_config)
        cg.add(var.set_current(sens))
    if voltage_config := config.get(CONF_VOLTAGE):
        sens = await sensor.new_sensor(voltage_config)
        cg.add(var.set_voltage(sens))
    if energy_consumed_config := config.get(CONF_ENERGY_CONSUMED):
        sens = await sensor.new_sensor(energy_consumed_config)
        cg.add(var.set_energy_consumed(sens))

    if real_power_config := config.get(CONF_POWER):
        sens = await sensor.new_sensor(real_power_config)
        cg.add(var.set_real_power(sens))
    if apparent_power_config := config.get(CONF_APPARENT_POWER):
        sens = await sensor.new_sensor(apparent_power_config)
        cg.add(var.set_apparent_power(sens))
    if reactive_power_config := config.get(CONF_REACTIVE_POWER):
        sens = await sensor.new_sensor(reactive_power_config)
        cg.add(var.set_reactive_power(sens))
    if power_factor_config := config.get(CONF_POWER_FACTOR):
        sens = await sensor.new_sensor(power_factor_config)
        cg.add(var.set_power_factor(sens))
    if phase_angle_config := config.get(CONF_PHASE_ANGLE):
        sens = await sensor.new_sensor(phase_angle_config)
        cg.add(var.set_phase_angle(sens))

    sens = await cg.get_variable(config[CONF_SENSOR_CURRENT])
    cg.add(var.set_current_source(sens))

    sens = await cg.get_variable(config[CONF_SENSOR_VOLTAGE])
    cg.add(var.set_voltage_source(sens))

    # sens = await cg.get_variable(config[CONF_ZERO_CROSSINGS])
    cg.add(var.set_zero_crossings(config[CONF_ZERO_CROSSINGS]))

    cg.add(var.set_sample_duration(config[CONF_SAMPLE_DURATION]))
