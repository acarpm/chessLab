#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// --- LED strip ---
#define LED_PIN     19
#define NUM_LEDS    64
#define BRIGHTNESS  15

// --- Hall sensor pins ---
#define HALL_DOUT    3    // A_OUT : A3144 open-collector, LOW = pièce présente
#define SEL_S0       0    // sélection colonne (mux)
#define SEL_S1       1
#define SEL_S2       2
#define DEC_A0      18    // adresse 74HC138 (sélection ligne)
#define DEC_A1      16
#define DEC_A2      17

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Câblage serpentin : rangées paires L→R, rangées impaires R→L
static int xy_to_index(int row, int col) {
    return (row % 2 == 0) ? row * 8 + col : row * 8 + (7 - col);
}

// Retourne true si une pièce (aimant) est détectée sur (row, col)
static bool read_hall(int row, int col) {
    int r = 7 - row;  // adresse inversée sur le 74HC138
    digitalWrite(DEC_A0, (r >> 0) & 1);
    digitalWrite(DEC_A1, (r >> 1) & 1);
    digitalWrite(DEC_A2, (r >> 2) & 1);
    digitalWrite(SEL_S0, (col >> 0) & 1);
    digitalWrite(SEL_S1, (col >> 1) & 1);
    digitalWrite(SEL_S2, (col >> 2) & 1);
    delayMicroseconds(10);
    return digitalRead(HALL_DOUT) == LOW;
}

void setup() {
    Serial.begin(115200);

    strip.begin();
    strip.setBrightness(BRIGHTNESS);

    // Test LED 2s : toutes les LEDs blanches pour confirmer que le board fonctionne
    strip.fill(strip.Color(200, 200, 200));
    strip.show();
    delay(2000);
    strip.fill(0);
    strip.show();

    pinMode(HALL_DOUT, INPUT);  // pull-up externe R1B (10k → 3.3V) sur la carte
    for (int p : {DEC_A0, DEC_A1, DEC_A2, SEL_S0, SEL_S1, SEL_S2})
        pinMode(p, OUTPUT);

    Serial.println("=== Échiquier — scan capteurs Hall ===");
}

void loop() {
    for (int row = 0; row < 8; row++) {
        int r = 7 - row;
        digitalWrite(DEC_A0, (r >> 0) & 1);
        digitalWrite(DEC_A1, (r >> 1) & 1);
        digitalWrite(DEC_A2, (r >> 2) & 1);
        digitalWrite(SEL_S0, 0);
        digitalWrite(SEL_S1, 0);
        digitalWrite(SEL_S2, 0);
        delayMicroseconds(10);
        int val = digitalRead(HALL_DOUT);
        Serial.printf("row%d col0 = %s\n", row, val == LOW ? "PIECE" : "vide");
    }
    Serial.println("---");
    delay(1000);
}
