#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN     19
#define NUM_LEDS    64      // 8x8 cases
#define BRIGHTNESS  10      // 0-255

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int xy_to_index(int row, int col) {
    // Câblage serpentin : rangées paires L→R, rangées impaires R→L
    if (row % 2 == 0)
        return row * 8 + col;
    else
        return row * 8 + (7 - col);
}

void fill_all(uint32_t color) {
    strip.fill(color);
    strip.show();
}

void test_couleurs() {
    Serial.println("[1] Test couleurs de base");
    struct { uint32_t color; const char* name; } palette[] = {
        { strip.Color(255, 0,   0),   "Rouge" },
        { strip.Color(0,   255, 0),   "Vert"  },
        { strip.Color(0,   0,   255), "Bleu"  },
        { strip.Color(255, 255, 255), "Blanc" },
    };
    for (auto& p : palette) {
        Serial.printf("    %s\n", p.name);
        fill_all(p.color);
        delay(1000);
    }
    fill_all(0);
    delay(300);
}

void test_individuel() {
    Serial.println("[2] Test LED par LED");
    fill_all(0);
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 200, 200));
        strip.show();
        delay(60);
        strip.setPixelColor(i, 0);
    }
    delay(300);
}

void test_echiquier() {
    Serial.println("[3] Motif échiquier");
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            bool claire = (row + col) % 2 == 0;
            uint32_t c = claire ? strip.Color(200, 160, 60) : strip.Color(30, 15, 0);
            strip.setPixelColor(xy_to_index(row, col), c);
        }
    }
    strip.show();
    delay(2500);
    fill_all(0);
    delay(300);
}

void test_arc_en_ciel() {
    Serial.println("[4] Arc-en-ciel");
    for (long hue = 0; hue < 3 * 65536L; hue += 256) {
        for (int i = 0; i < NUM_LEDS; i++) {
            uint32_t c = strip.gamma32(strip.ColorHSV(hue + (i * 65536L / NUM_LEDS)));
            strip.setPixelColor(i, c);
        }
        strip.show();
        delay(10);
    }
    fill_all(0);
    delay(300);
}

void test_rangees() {
    Serial.println("[5] Balayage rangées");
    fill_all(0);
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++)
            strip.setPixelColor(xy_to_index(row, col), strip.Color(180, 0, 180));
        strip.show();
        delay(250);
        for (int col = 0; col < 8; col++)
            strip.setPixelColor(xy_to_index(row, col), 0);
    }
    delay(300);
}

void setup() {
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 3000)
        delay(10);

    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    fill_all(0);

    Serial.println("=== Échiquier intelligent — Test LEDs WS2812B ===");
    Serial.printf("GPIO: %d | LEDs: %d | Luminosité: %d\n", LED_PIN, NUM_LEDS, BRIGHTNESS);
}

void loop() {
    test_couleurs();
    test_individuel();
    test_echiquier();
    test_arc_en_ciel();
    test_rangees();
    delay(1000);
}
