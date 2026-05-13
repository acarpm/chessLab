#pragma once
#include "board_config.h"

void web_init();
void web_update();
void web_broadcast_state(int dev[BOARD_SIZE][BOARD_SIZE]);
