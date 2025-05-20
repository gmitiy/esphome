import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_DIRECTION, DEVICE_CLASS_MOVING

from . import CONF_PAJ7620_ID, PAJ7620

DEPENDENCIES = ["paj7620"]

DIRECTIONS = [
    "up",
    "down",
    "left",
    "right",
    "push",
    "poll",
    "clockwise",
    "anti_clockwise",
    "wave",
]

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    device_class=DEVICE_CLASS_MOVING
).extend(
    {
        cv.GenerateID(CONF_PAJ7620_ID): cv.use_id(PAJ7620),
        cv.Required(CONF_DIRECTION): cv.one_of(*DIRECTIONS, lower=True),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PAJ7620_ID])
    var = await binary_sensor.new_binary_sensor(config)
    func = getattr(hub, f"set_{config[CONF_DIRECTION]}_direction_binary_sensor")
    cg.add(func(var))
