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
  mutable uint32_t last_dump_ms;

  HallSquareState grid[BOARD_SIZE][BOARD_SIZE];
  uint64_t squares;
  void write_square(uint8_t col, uint8_t row, bool value);
  bool read_square(uint8_t col, uint8_t row) const;

  HallMatrix();
  void updateCell(uint8_t row, uint8_t col);

public:
  static HallMatrix &getInstance();
  void update();
  void getSquares(bool (&result)[BOARD_SIZE][BOARD_SIZE]) const;

  void dump() const;
};

#define hallMatrix HallMatrix::getInstance()
