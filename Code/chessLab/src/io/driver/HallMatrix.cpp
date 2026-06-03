#include "io/driver/HallMatrix.h"
#include "io/hal/HallSensor.h"

HallMatrix::HallMatrix()
{
  for (int row = 0; row < BOARD_SIZE; row++)
    for (int col = 0; col < BOARD_SIZE; col++)
      squares[row][col] = false;
}

HallMatrix &HallMatrix::getInstance()
{
  static HallMatrix instance;
  return instance;
}

void HallMatrix::updateCell(int row, int col)
{
  HallSquareState &cell = grid[row][col];
  int value = hallSensor.read(row, col);
  cell.raw = value;

  bool new_occupied = abs(value - BASE_VALUE) > OFFSET;

  if (new_occupied != cell.occupied)
  {
    cell.last_change = millis();
    cell.stable = false;
    cell.occupied = new_occupied;
  }

  if (millis() - cell.last_change > STABLE_TIME_MS)
  {
    cell.stable = true;
    squares[row][col] = cell.occupied;
  }
}

void HallMatrix::update()
{
  for (int row = 0; row < BOARD_SIZE; row++)
    for (int col = 0; col < BOARD_SIZE; col++)
      updateCell(row, col);
}

bool HallMatrix::isOccupied(int row, int col)
{
  return squares[row][col];
}

void HallMatrix::getSquares(bool (&result)[BOARD_SIZE][BOARD_SIZE])
{
  for (int row = 0; row < BOARD_SIZE; row++)
    for (int col = 0; col < BOARD_SIZE; col++)
      result[row][col] = squares[row][col];
}
