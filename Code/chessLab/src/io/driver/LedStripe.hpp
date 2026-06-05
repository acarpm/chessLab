#pragma once
#include <Adafruit_NeoPixel.h>
#include "const/board_config.hpp"
#include "const/gpio_pins.hpp"

class LedStripe
{
private:
  static Adafruit_NeoPixel strip;

  LedStripe();
  uint8_t to_index(uint8_t row, uint8_t col);

public:
  static LedStripe &getInstance();
  void begin();

  void show_active(bool (&squares)[BOARD_SIZE][BOARD_SIZE]);
};

#define ledStripe LedStripe::getInstance()
