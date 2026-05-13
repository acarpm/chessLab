#include <Arduino.h>
#include "board_config.h"
#include "hal_sensors.h"
#include "hall_sensor.h"
#include "logger.h"

static int baseline[BOARD_SIZE][BOARD_SIZE];

void hall_calibrate() {
    LOG("I    1    2    3    4    5    6    7    8");
    for (int row = 0; row < BOARD_SIZE; row++) {
        LOG("%c  ", 'A' + row);
        for (int col = 0; col < BOARD_SIZE; col++) {
            baseline[row][col] = sensors_read_raw(row, col);
            LOG("%4d ", baseline[row][col]);
        }
        LOG("\n");
    }
    LOG("\n");
}

void hall_scan(int dev[BOARD_SIZE][BOARD_SIZE]) {
    for (int row = 0; row < BOARD_SIZE; row++)
        for (int col = 0; col < BOARD_SIZE; col++) {
            int raw = sensors_read_raw(row, col);
            int d   = abs(raw - baseline[row][col]);
            dev[row][col] = (d < HALL_DEAD_ZONE) ? 0 : d;
        }
}

void hall_dump_grid() {
    LOG("    1    2    3    4    5    6    7    8");
    for (int row = 0; row < BOARD_SIZE; row++) {
        LOG("%c  ", 'A' + row);
        for (int col = 0; col < BOARD_SIZE; col++) {
            int raw = sensors_read_raw(row, col);
            int d   = abs(raw - baseline[row][col]);
            LOG("%4d ", d);
        }
        LOG("\n");
    }
    LOG("\n");
}

int hall_get_baseline(int row, int col) {
    return baseline[row][col];
}
