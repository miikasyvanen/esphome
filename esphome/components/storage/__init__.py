import esphome.codegen as cg
from esphome.components import web_server
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_WEB_SERVER
from esphome.core import CORE, CoroPriority, coroutine_with_priority
from esphome.core.entity_helpers import setup_entity

CODEOWNERS = ["@miikasyvanen"]
IS_PLATFORM_COMPONENT = True

storage_ns = cg.esphome_ns.namespace("storage")
Storage = storage_ns.class_("storage", cg.EntityBase)
StoragePtr = Storage.operator("ptr")

STORAGE_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(Storage),
        }
    ),
)


async def setup_storage_core_(var, config):
    await setup_entity(var, config, "storage")

    if web_server_config := config.get(CONF_WEB_SERVER):
        await web_server.add_entity_config(var, web_server_config)


async def register_storage(var, config):
    if not CORE.has_id(config[CONF_ID]):
        var = cg.Pvariable(config[CONF_ID], var)
    cg.add(cg.App.register_storage(var))
    CORE.register_platform_component("storage", var)
    await setup_storage_core_(var, config)


async def new_storage(config, *args):
    var = cg.new_Pvariable(config[CONF_ID], *args)
    await register_storage(var, config)
    return var


@coroutine_with_priority(CoroPriority.CORE)
async def to_code(config):
    cg.add_global(storage_ns.using)
