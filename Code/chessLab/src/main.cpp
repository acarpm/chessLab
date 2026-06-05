#include "const/board_config.h"
#include "io/driver/HallMatrix.h"
#include "io/driver/LedStripe.h"

void init_serial()
{
  Serial.begin(115200);
  for (int i = 0; i < 30 && !Serial; ++i)
  {
    delay(100);
  }
  delay(200);
  Serial.setTimeout(30000);
}

void setup()
{
  init_serial();
  Serial.print("ChessLab Starting...\n");
  delay(100);

  ledStripe.begin();
}

void loop()
{
  hallMatrix.update();

  bool board[BOARD_SIZE][BOARD_SIZE];
  hallMatrix.getSquares(board);

  ledStripe.show_active(board);
}
