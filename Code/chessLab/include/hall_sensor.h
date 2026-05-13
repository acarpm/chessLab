#pragma once

void hall_calibrate();
void hall_scan(int dev[8][8]);
void hall_dump_grid();
int  hall_get_baseline(int row, int col);
