#include "io/driver/HallMatrix.hpp"
#include "io/hal/HallSensor.hpp"

HallMatrix::HallMatrix() : squares(0), last_dump_ms(0) {}

void HallMatrix::write_square(uint8_t col, uint8_t row, bool value)
{
  uint8_t bit = row * BOARD_SIZE + col;
  if (value)
    squares |= (1ULL << bit);
  else
    squares &= ~(1ULL << bit);
}

bool HallMatrix::read_square(uint8_t col, uint8_t row) const
{
  return (squares >> (row * BOARD_SIZE + col)) & 1;
}

HallMatrix &HallMatrix::getInstance()
{
  static HallMatrix instance;
  return instance;
}

void HallMatrix::updateCell(uint8_t col, uint8_t row)
{
  HallSquareState &cell = grid[col][row];
  uint16_t value = hallSensor.read(col, row);
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
    write_square(col, row, cell.occupied);
  }
}

void HallMatrix::update()
{
  for (uint8_t row = 0; row < BOARD_SIZE; row++)
    for (uint8_t col = 0; col < BOARD_SIZE; col++)
      updateCell(col, row);
}

void HallMatrix::getSquares(bool (&result)[BOARD_SIZE][BOARD_SIZE]) const
{
  for (uint8_t col = 0; col < BOARD_SIZE; col++)
    for (uint8_t row = 0; row < BOARD_SIZE; row++)
      result[col][row] = read_square(col, row);
}

void HallMatrix::dump() const
{
  if (last_dump_ms != 0 && millis() - last_dump_ms < 1000)
    return;

  last_dump_ms = millis();

  bool board[BOARD_SIZE][BOARD_SIZE];
  hallMatrix.getSquares(board);

  char out[512];
  size_t pos = 0;

  pos += snprintf(out + pos, sizeof(out) - pos, "\n  A B C D E F G H\n");

  for (int8_t row = BOARD_SIZE - 1; row >= 0; row--)
  {
    pos += snprintf(out + pos, sizeof(out) - pos, "%d ", row + 1);

    for (uint8_t col = 0; col < BOARD_SIZE; col++)
    {
      pos += snprintf(out + pos, sizeof(out) - pos, "%c ", board[col][row] ? 'X' : '.');
    }

    pos += snprintf(out + pos, sizeof(out) - pos, "\n");
  }

  Serial.print(out);
}
