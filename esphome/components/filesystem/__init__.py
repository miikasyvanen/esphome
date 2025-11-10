import esphome.codegen as cg

# from esphome.components import web_server
import esphome.config_validation as cv
from esphome.const import CONF_ID

# from esphome.const import CONF_BASE_PATH, CONF_ID, CONF_LABEL, CONF_PARTITIONS
from esphome.core import CORE, CoroPriority, coroutine_with_priority
from esphome.core.entity_helpers import setup_entity

# from esphome.util import Registry

CODEOWNERS = ["@miikasyvanen"]
IS_PLATFORM_COMPONENT = True

filesystem_ns = cg.esphome_ns.namespace("filesystem")
Filesystem = filesystem_ns.class_("filesystem", cg.EntityBase)
FilesystemPtr = Filesystem.operator("ptr")

FILESYSTEM_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(Filesystem),
        }
    ),
)


async def setup_filesystem_core_(var, config):
    await setup_entity(var, config, "filesystem")


async def register_filesystem(var, config):
    if not CORE.has_id(config[CONF_ID]):
        var = cg.Pvariable(config[CONF_ID], var)
    cg.add(cg.App.register_filesystem(var))
    CORE.register_platform_component("filesystem", var)
    await setup_filesystem_core_(var, config)


async def new_filesystem(config, *args):
    var = cg.new_Pvariable(config[CONF_ID], *args)
    await register_filesystem(var, config)
    return var


@coroutine_with_priority(CoroPriority.CORE)
async def to_code(config):
    cg.add_global(filesystem_ns.using)
