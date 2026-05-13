#pragma once

void sensors_init();
void sensors_select_row(int row);
void sensors_select_col(int col);
int  sensors_read_raw(int row, int col);
