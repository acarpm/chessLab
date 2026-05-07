#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN          19
#define NUM_LEDS         64
#define BRIGHTNESS       15
#define HALL_DOUT         3
#define SEL_S0            0
#define SEL_S1            1
#define SEL_S2            2
#define DEC_A0           18
#define DEC_A1           16
#define DEC_A2           17
#define HALL_RANGE       500
#define HALL_DEAD_ZONE    40
#define COL_OFFSET         1  // ajuste si les LEDs sont décalées par rapport aux capteurs

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

static int baseline[8][8];

// --- Utilitaires bas niveau ---

static int xy_to_index(int row, int col) {
    return row * 8 + col;
}

static void write_addr(int p0, int p1, int p2, int val) {
    digitalWrite(p0, val & 1);
    digitalWrite(p1, (val >> 1) & 1);
    digitalWrite(p2, (val >> 2) & 1);
}

static int read_hall_raw(int row, int col) {
    write_addr(DEC_A0, DEC_A1, DEC_A2, row);
    write_addr(SEL_S0, SEL_S1, SEL_S2, col);
    delayMicroseconds(200);
    return analogRead(HALL_DOUT);
}

// --- Calibration ---

static void calibrate() {
    Serial.println("Calibration plateau vide...");
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            baseline[row][col] = read_hall_raw(row, col);
}

// --- Debug série ---

static void dump_grid() {
    Serial.println("    1    2    3    4    5    6    7    8");
    for (int row = 0; row < 8; row++) {
        Serial.printf("%c  ", 'A' + row);
        for (int col = 0; col < 8; col++)
            Serial.printf("%4d ", read_hall_raw(row, col) - baseline[row][col]);
        Serial.println();
    }
    Serial.println();
}

// --- Scan des déviations ---

static void scan_deviations(int dev[8][8]) {
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++) {
            int d = abs(read_hall_raw(row, col) - baseline[row][col]);
            dev[row][col] = (d < HALL_DEAD_ZONE) ? 0 : d;
        }
}

// --- Affichage LEDs ---

static void update_leds(int dev[8][8]) {
    strip.clear();
    bool any = false;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int d = dev[row][col];
            if (d == 0) continue;
            if (row > 0 && dev[row-1][col] >= d) continue;
            if (row < 7 && dev[row+1][col] >= d) continue;
            if (col > 0 && dev[row][col-1] >= d) continue;
            if (col < 7 && dev[row][col+1] >= d) continue;

            int led_col = col - COL_OFFSET;
            if (led_col < 0 || led_col > 7) continue;

            if (!any) { Serial.print("Active: "); any = true; }
            Serial.printf("%c%d(%d) ", 'A' + row, led_col + 1, d);

            int brightness = map(constrain(d, 0, HALL_RANGE), 0, HALL_RANGE, 0, 255);
            strip.setPixelColor(xy_to_index(row, led_col), strip.Color(brightness, 0, 0));
        }
    }

    if (any) Serial.println();
    strip.show();
}

// --- Test manuel LEDs ---

static void test_led_manual() {
    Serial.print("Ligne (0-7): ");
    while (!Serial.available());
    int row = Serial.parseInt();

    Serial.print("Colonne (0-7): ");
    while (!Serial.available());
    int col = Serial.parseInt();

    strip.clear();

    if (row < 0 || row > 7 || col < 0 || col > 7) {
        Serial.println("Valeur invalide.");
        strip.show();
        return;
    }

    strip.setPixelColor(xy_to_index(row, col), strip.Color(0, 200, 0));
    strip.show();
    Serial.printf("LED %c%d allumée (index %d)\n", 'A' + row, col + 1, xy_to_index(row, col));
}

// --- Setup / Loop ---

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(30000);

    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.fill(strip.Color(200, 200, 200));
    strip.show();
    delay(2000);
    strip.fill(0);
    strip.show();

    pinMode(HALL_DOUT, INPUT);
    for (int p : {DEC_A0, DEC_A1, DEC_A2, SEL_S0, SEL_S1, SEL_S2})
        pinMode(p, OUTPUT);

    calibrate();
    Serial.println("=== ChessLab — SS49E scan ===");
}

void loop() {
    test_led_manual();
}
