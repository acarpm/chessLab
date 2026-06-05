#pragma once

#include <Arduino.h>
#include "const/board_config.h"

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
  void updateCell(int row, int col);

public:
  static HallMatrix &getInstance();
  void update();
  bool isOccupied(int row, int col);
  void getSquares(bool (&result)[BOARD_SIZE][BOARD_SIZE]);
};

#define hallMatrix HallMatrix::getInstance()
