#include "esphome/core/log.h"
#include "acw02.h"

namespace esphome {
namespace acw02 {



static const char *TAG = "empty_binary_sensor.binary_sensor";

void EmptyBinarySensor::setup() {
    
}
  
void EmptyBinarySensor::update() {

}

void EmptyBinarySensor::dump_config() {
    ESP_LOGCONFIG(TAG, "Custom binary sensor");
}

} //namespace empty_binary_output
} //namespace esphome