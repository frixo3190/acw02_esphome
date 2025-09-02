#pragma once





#include <cstdint>
#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace acw02 {

class EmptyBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void setup() override;
  void dump_config() override;
};


} //namespace empty_binary_output
} //namespace esphome