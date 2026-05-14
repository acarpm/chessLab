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

// Animation non-bloquante : balayage A→H sur cols 1-2, H→A sur cols 7-8
// Orange = case vide à remplir, Vert = pièce correctement posée
void leds_animate_setup(int dev[BOARD_SIZE][BOARD_SIZE]) {
    static uint32_t last_ms = 0;
    static int step = 0;  // 0-17 : 0-7 montée, 8-10 pause plein, 11-17 descente

    uint32_t now = millis();
    uint32_t interval = (step >= 8 && step <= 10) ? 400 : 280;  // pause en bout de course
    if (now - last_ms >= interval) {
        last_ms = now;
        step = (step + 1) % 18;
    }

    // step 0-7 → sweep 0-7, step 8-10 → sweep 7 (pause), step 11-17 → sweep 6-0
    int sweep;
    if (step <= 7)       sweep = step;
    else if (step <= 10) sweep = 7;
    else                 sweep = 17 - step;

    strip.clear();

    for (int r = 0; r < BOARD_SIZE; r++) {
        if (r <= sweep) {
            strip.setPixelColor(leds_xy_to_index(r, 0),
                (dev[r][0] >= THRESHOLD) ? strip.Color(0, 180, 30) : strip.Color(180, 100, 0));
            strip.setPixelColor(leds_xy_to_index(r, 1),
                (dev[r][1] >= THRESHOLD) ? strip.Color(0, 180, 30) : strip.Color(180, 100, 0));
        }
        if (r >= BOARD_SIZE - 1 - sweep) {
            strip.setPixelColor(leds_xy_to_index(r, 6),
                (dev[r][6] >= THRESHOLD) ? strip.Color(0, 180, 30) : strip.Color(180, 100, 0));
            strip.setPixelColor(leds_xy_to_index(r, 7),
                (dev[r][7] >= THRESHOLD) ? strip.Color(0, 180, 30) : strip.Color(180, 100, 0));
        }
    }

    strip.show();
}

// Deux flashs verts pour confirmer la position de départ (bloquant ~1.1 s)
void leds_flash_green() {
    for (int i = 0; i < 2; i++) {
        strip.fill(strip.Color(0, 200, 30));
        strip.show();
        delay(350);
        strip.clear();
        strip.show();
        delay(200);
    }
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
