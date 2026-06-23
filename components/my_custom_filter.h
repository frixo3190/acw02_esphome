#pragma once
#include "esphome/components/uart/uart.h"

class FilteredUART : public esphome::uart::UARTDevice {
public:
  int read() override {
    int b = UARTDevice::read();
    // Si on lit 0x55, vérifier si le suivant est aussi 0x55
    // Dans ce cas, consommer le premier et retourner le second
    if (b == 0x55 && available() && peek() == 0x55) {
      UARTDevice::read(); // consomme le 55 parasite
    }
    return b;
  }
};