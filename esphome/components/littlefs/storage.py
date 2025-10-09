import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_BASE_PATH, CONF_ID, CONF_LABEL, CONF_PARTITIONS
from esphome.util import Registry

CODEOWNERS = ["@miikasyvanen"]

littlefs_ns = cg.esphome_ns.namespace("littlefs")
LittleFSComponent = littlefs_ns.class_("LittleFSComponent", cg.Component)

LittleFS = littlefs_ns.class_("LittleFS", cg.Component)

PARTITIONS_REGISTRY = Registry()
# validate_partitions = cv.validate_registry("partitions", PARTITIONS_REGISTRY)

Partitions = littlefs_ns.class_("Partitions")
Partition = littlefs_ns.class_("Partition", Partitions)


def validate_partitions(partition):
    print("Validate partition:")
    if "id" in partition:
        print("Partition ID: " + partition["id"])
    else:
        raise cv.Invalid("Partition id must be defined")
    if "base_path" in partition:
        print("Partition base path: " + partition["base_path"])
    else:
        raise cv.Invalid("Partition base path must be defined")
    if "label" in partition:
        print("Partition label: " + partition["label"])
    else:
        raise cv.Invalid("Partition label must be defined")

    return partition


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(LittleFS),
            # cv.Required(CONF_PARTITIONS): validate_partitions,
            cv.Optional(CONF_PARTITIONS, default=[]): cv.ensure_list(
                validate_partitions
            ),
        }
    ),
)


def validate_partition(config):
    return config


PARTITION_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Optional(CONF_ID, default="Default"): cv.string,
            cv.Optional(CONF_BASE_PATH, default="LittleFS"): cv.string,
            cv.Optional(CONF_LABEL, default="LittleFS"): cv.string,
        }
    ),
    validate_partition,
)


@PARTITIONS_REGISTRY.register("partition", Partition, PARTITION_SCHEMA)
async def partition_to_code(config, partition_id):
    return cg.new_Pvariable(
        partition_id,
        config[CONF_ID],
        config[CONF_BASE_PATH],
        config[CONF_LABEL],
    )


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    if CONF_PARTITIONS in config:
        #        conf = config[CONF_PARTITIONS]

        for partition in config.get(CONF_PARTITIONS, []):
            if CONF_ID in partition:
                print("ID: " + partition[CONF_ID])
            if CONF_BASE_PATH in partition:
                print("BASE PATH: " + partition[CONF_BASE_PATH])
            if CONF_LABEL in partition:
                print("LABEL: " + partition[CONF_LABEL])


#          cg.add(var.setMode(conf[CONF_MODE]))
