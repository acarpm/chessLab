#include "const/board_config.h"
#include "io/driver/HallMatrix.h"

void init_serial()
{
  Serial.begin(115200);
  for (int i = 0; i < 30 && !Serial; ++i)
  {
    delay(100);
  }
  delay(200);
  Serial.setTimeout(30000);
  Serial.print('\n');
}

void setup()
{
  init_serial();
  Serial.print("ChessLab Starting...\n");
}

void loop()
{
  Serial.print('\n');
  hallMatrix.update();

  bool squares[BOARD_SIZE][BOARD_SIZE];
  hallMatrix.getSquares(squares);

  for (int row = 0; row < BOARD_SIZE; row++)
  {
    char line[BOARD_SIZE * 2 + 1];
    for (int col = 0; col < BOARD_SIZE; col++)
    {
      line[col * 2] = squares[row][col] ? 'X' : '.';
      line[col * 2 + 1] = ' ';
    }
    line[BOARD_SIZE * 2] = '\0';
    Serial.printf("|%s\n", line);
  }
  Serial.print("\n");
}
