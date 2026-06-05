#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "io/driver/LedStripe.hpp"

Adafruit_NeoPixel LedStripe::strip(BOARD_SQUARES, LED_PIN, NEO_GRB + NEO_KHZ800);

LedStripe::LedStripe()
{
}

int LedStripe::to_index(int row, int col)
{
  return row * BOARD_SIZE + col;
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
  for (int row = 0; row < BOARD_SIZE; row++)
  {
    for (int col = 0; col < BOARD_SIZE; col++)
    {
      int index = to_index(row, col);
      if (board[row][col])
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
