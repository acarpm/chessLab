#pragma once

#include <Arduino.h>
#include "const/board_config.hpp"

struct HallSquareState
{
  int raw = 0;
  bool occupied = false;
  bool stable = false;
  uint32_t last_change = 0;
};

class HallMatrix
{
private:
  static const int BASE_VALUE = 1600;
  static const int OFFSET = 150;
  static const uint32_t STABLE_TIME_MS = 300;

  HallSquareState grid[BOARD_SIZE][BOARD_SIZE];
  bool squares[BOARD_SIZE][BOARD_SIZE];

  HallMatrix();
  void updateCell(uint8_t row, uint8_t col);

public:
  static HallMatrix &getInstance();
  void update();
  void getSquares(bool (&result)[BOARD_SIZE][BOARD_SIZE]);

  void dump();
};

#define hallMatrix HallMatrix::getInstance()
