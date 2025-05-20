import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]

CONF_PAJ7620_ID = "paj7620_id"
CONF_REPORT_MODE = "report_mode"
CONF_GESTURE_ENTERY_TIME = "gesture_entery_time"
CONF_GESTURE_QUIT_TIME = "gesture_quit_time"


REPORT_MODE = {
    "far_240fps": 53,
    "far_120fps": 183,
    "near_240fps": 18,
    "near_120fps": 148,
}
PAJ7620_I2C_ADDRES = 0x73

paj7620_nds = cg.esphome_ns.namespace("paj7620")
PAJ7620 = paj7620_nds.class_("PAJ7620", cg.Component, i2c.I2CDevice)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(PAJ7620),
        cv.Optional(CONF_REPORT_MODE, "near_240fps"): cv.enum(REPORT_MODE, lower=True),
        cv.Optional(CONF_GESTURE_ENTERY_TIME, default=800): cv.int_range(
            min=0, max=8388607
        ),
        cv.Optional(CONF_GESTURE_QUIT_TIME, default=1000): cv.int_range(
            min=0, max=8388607
        ),
    }
).extend(i2c.i2c_device_schema(PAJ7620_I2C_ADDRES))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add(var.set_report_mode(config[CONF_REPORT_MODE]))
    cg.add(var.set_gesture_entery_time(config[CONF_GESTURE_ENTERY_TIME]))
    cg.add(var.set_gesture_quit_time(config[CONF_GESTURE_QUIT_TIME]))
