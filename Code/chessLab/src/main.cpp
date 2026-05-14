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
    static int start_ok_frames = 0;

    logger_update();

    // WiFi d'abord : vider la file d'envoi/réception avant de toucher l'ADC
    web_update();
    ota_update();

    hall_scan(dev);  // ADC après le flush WiFi → moins d'interférences

    if (game_get_phase() == GAME_SETUP) {
        leds_animate_setup(dev);

        if (game_check_start_position(dev)) {
            if (++start_ok_frames >= 5) {
                game_set_phase(GAME_PLAYING);
                LOG("=== Position de départ validée — jeu commence ===\n");
                leds_flash_green();
                start_ok_frames = 0;
            }
        } else {
            start_ok_frames = 0;
        }
    } else {
        leds_update(dev);
    }

    game_update(dev);
    web_broadcast_state(dev);  // mis en file, sera transmis au prochain web_update()
}
