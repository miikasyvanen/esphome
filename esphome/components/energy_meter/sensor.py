import esphome.codegen as cg
from esphome.components import sensor, voltage_sampler
import esphome.config_validation as cv
from esphome.const import (
    CONF_CURRENT,
    CONF_ENERGY_CONSUMED,
    CONF_ID,
    CONF_SENSOR_CURRENT,
    CONF_SENSOR_VOLTAGE,
    CONF_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_KILOWATT_HOURS,
    UNIT_VOLT,
)

AUTO_LOAD = ["voltage_sampler"]
CODEOWNERS = ["@miikasyvanen"]

CONF_SAMPLE_DURATION = "sample_duration"

energy_meter_ns = cg.esphome_ns.namespace("energy_meter")
EnergyMeterSensor = energy_meter_ns.class_(
    "EnergyMeterSensor", sensor.Sensor, cg.PollingComponent
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ENERGY_CONSUMED): sensor.sensor_schema(
                EnergyMeterSensor,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ).extend(
                {
                    cv.Required(CONF_SENSOR_CURRENT): cv.use_id(
                        voltage_sampler.VoltageSampler
                    ),
                    cv.Required(CONF_SENSOR_VOLTAGE): cv.use_id(
                        voltage_sampler.VoltageSampler
                    ),
                    cv.Optional(
                        CONF_SAMPLE_DURATION, default="200ms"
                    ): cv.positive_time_period_milliseconds,
                }
            ),
        }
    )
    .extend(
        {
            cv.GenerateID(CONF_ID): cv.use_id(EnergyMeterSensor),
        }
    )
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
        sens = await cg.get_variable(energy_consumed_config[CONF_SENSOR_CURRENT])
        cg.add(var.set_current_source(sens))
        sens = await cg.get_variable(energy_consumed_config[CONF_SENSOR_VOLTAGE])
        cg.add(var.set_voltage_source(sens))
        cg.add(var.set_sample_duration(energy_consumed_config[CONF_SAMPLE_DURATION]))
