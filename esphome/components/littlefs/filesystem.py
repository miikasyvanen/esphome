import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_BASE_PATH, CONF_ID, CONF_LABEL

CODEOWNERS = ["@miikasyvanen"]

littlefs_ns = cg.esphome_ns.namespace("littlefs")
LittleFSComponent = littlefs_ns.class_("LittleFSComponent", cg.Component)

LittleFS = littlefs_ns.class_("LittleFS", cg.Component)

Partitions = littlefs_ns.class_("Partitions")
Partition = littlefs_ns.class_("Partition", Partitions)


def validate_partition(config):
    return config


def _validate(config):
    print("Validate partition:")

    if CONF_ID in config:
        id = config.get(CONF_ID)
        print("Partition ID: " + id)
    else:
        raise cv.Invalid("Partition id must be defined")
    if CONF_BASE_PATH in config:
        base_path = config.get(CONF_BASE_PATH)
        print("Partition base path: " + base_path)
    else:
        raise cv.Invalid("Partition base path must be defined")
    if CONF_LABEL in config:
        label = config.get(CONF_LABEL)
        print("Partition label: " + label)
    else:
        raise cv.Invalid("Partition label must be defined")

    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(LittleFS),
            cv.Optional(CONF_BASE_PATH, default="/littlefs"): cv.string,
            cv.Optional(CONF_LABEL, default="littlefs"): cv.string,
        }
    ),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # if CONF_ID in config:
    #    partition_id = config[CONF_ID] ###not used
    #        print("ID: " + partition_id)
    # if CONF_BASE_PATH in config:
    #    base_path = config[CONF_BASE_PATH] ###not used
    #        print("BASE PATH: " + base_path)
    # if CONF_LABEL in config:
    #    label = config[CONF_LABEL] ###not used


#        print("LABEL: " + label)
