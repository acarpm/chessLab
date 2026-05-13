#include <Arduino.h>
#include "board_config.h"
#include "logger.h"
#include "hal_sensors.h"
#include "hall_sensor.h"
#include "led_matrix.h"
#include "chess_game.h"
#include "web_interface.h"
#include "ota.h"

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(30000);
    sensors_init();
    leds_init();
    hall_calibrate();
    game_init();
    ota_init();       // WiFi doit être up avant logger_init()
    logger_init();
    web_init();
    LOG("=== ChessLab — INIT sequence OK ===\n");
}

void loop() {
    static int dev[BOARD_SIZE][BOARD_SIZE];
    logger_update();
    hall_scan(dev);
    leds_update(dev);
    game_update(dev);
    web_broadcast_state(dev);
    web_update();
    ota_update();
}
