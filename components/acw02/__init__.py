import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID

empty_binary_sensor_ns = cg.esphome_ns.namespace('acw02')
empty_binary_sensor_ns = empty_binary_sensor_ns.class_('EmptyBinarySensor', binary_sensor.BinarySensor, cg.Component)

CONFIG_SCHEMA = binary_sensor.BINARY_SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(empty_binary_sensor_ns)
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)