#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "io/driver/LedStripe.hpp"

Adafruit_NeoPixel LedStripe::strip(BOARD_SQUARES, LED_PIN, NEO_GRB + NEO_KHZ800);

LedStripe::LedStripe()
{
}

uint8_t LedStripe::to_index(uint8_t col, uint8_t row)
{
  return col * BOARD_SIZE + row;
}

LedStripe &LedStripe::getInstance()
{
  static LedStripe instance;
  return instance;
}

void LedStripe::begin()
{
  Serial.print("Initializing LED Stripe...\n");
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.fill(strip.Color(200, 200, 200));
  strip.show();
  delay(2000);
  strip.fill(0);
  strip.show();
}

void LedStripe::show_active(bool (&board)[BOARD_SIZE][BOARD_SIZE])
{
  for (uint8_t row = 0; row < BOARD_SIZE; row++)
  {
    for (uint8_t col = 0; col < BOARD_SIZE; col++)
    {
      uint8_t index = to_index(col, row);
      if (board[col][row])
      {
        strip.setPixelColor(index, strip.Color(0, 255, 0));
      }
      else
      {
        strip.setPixelColor(index, strip.Color(0, 0, 0));
      }
    }
  }
  strip.show();
}
