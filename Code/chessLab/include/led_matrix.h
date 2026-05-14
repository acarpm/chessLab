#pragma once
#include <stdint.h>
#include "board_config.h"

void leds_init();
void leds_update(int dev[BOARD_SIZE][BOARD_SIZE]);
void leds_animate_setup(int dev[BOARD_SIZE][BOARD_SIZE]);
void leds_flash_green();
void leds_set_pixel(int row, int col, uint32_t color);
void leds_clear();
void leds_show();
int  leds_xy_to_index(int row, int col);
int  leds_xy_to_index_serpentine(int row, int col);
