#pragma once
#include <Adafruit_NeoPixel.h>
#include "const/board_config.hpp"
#include "const/gpio_pins.hpp"

class LedStripe
{
private:
  static Adafruit_NeoPixel strip;

  LedStripe();
  int to_index(int row, int col);

public:
  static LedStripe &getInstance();
  void begin();

  void show_active(bool (&squares)[BOARD_SIZE][BOARD_SIZE]);
};

#define ledStripe LedStripe::getInstance()
