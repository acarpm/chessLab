#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "board_config.h"
#include "led_matrix.h"
#include "logger.h"

static Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Réservé pour futur effet de fondu :
// static float fade[BOARD_SIZE][BOARD_SIZE];
// Décroissance : multiplier fade[row][col] par FADE_DECAY à chaque frame,
// puis utiliser fade[row][col] comme facteur de luminosité.

void leds_init() {
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.fill(strip.Color(200, 200, 200));
    strip.show();
    delay(2000);
    strip.fill(0);
    strip.show();
}

int leds_xy_to_index(int row, int col) {
    return row * BOARD_SIZE + col;
}

int leds_xy_to_index_serpentine(int row, int col) {
    if (row % 2 == 0)
        return row * BOARD_SIZE + col;
    else
        return row * BOARD_SIZE + (BOARD_SIZE - 1 - col);
}

void leds_update(int dev[BOARD_SIZE][BOARD_SIZE]) {
    strip.clear();
    bool any = false;

    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            int d = dev[row][col];
            if (d < THRESHOLD) continue;

            int led_col = col - COL_OFFSET;
            if (led_col < 0 || led_col >= BOARD_SIZE) continue;

            if (!any) { LOG("Active: "); any = true; }
            LOG("%c%d(%d) ", 'A' + row, led_col + 1, d);

            int brightness = map(constrain(d, 0, HALL_RANGE), 0, HALL_RANGE, 0, 255);
            strip.setPixelColor(leds_xy_to_index(row, led_col), strip.Color(brightness, 0, 0));
        }
    }

    if (any) LOG("\n");
    strip.show();
}

void leds_set_pixel(int row, int col, uint32_t color) {
    strip.setPixelColor(leds_xy_to_index(row, col), color);
}

void leds_clear() {
    strip.clear();
}

void leds_show() {
    strip.show();
}
