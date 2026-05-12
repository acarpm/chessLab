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
#define FADE_DECAY      0.97f // per-scan decay: ~2s fade at 50ms/scan

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

static int   baseline[8][8];
static float fade[8][8];

// --- Utilitaires bas niveau ---

static int xy_to_index(int row, int col) {
    return row * 8 + col;
}

static void write_addr(int p0, int p1, int p2, int val) {
    digitalWrite(p0, val & 1);
    digitalWrite(p1, (val >> 1) & 1);
    digitalWrite(p2, (val >> 2) & 1);
}

static void select_row(int row) {
    digitalWrite(DEC_A0, (row >> 0) & 1);
    digitalWrite(DEC_A1, (row >> 1) & 1);
    digitalWrite(DEC_A2, (row >> 2) & 1);
}

static void select_col(int col) {
    // SEL_S1 = group select (bit 2: 0=rows A-D, 1=rows E-H)
    // SEL_S0 = within-group bit 1 (MSB)
    // SEL_S2 = within-group bit 0 (LSB)
    digitalWrite(SEL_S1, (col >> 2) & 1);
    digitalWrite(SEL_S0, (col >> 1) & 1);
    digitalWrite(SEL_S2, (col >> 0) & 1);
}

static int read_hall_raw(int row, int col) {
    select_row(row);
    select_col(col);
    delayMicroseconds(500);
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += analogRead(HALL_DOUT);
    return sum / 8;
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

static void test_hall_manual() {
    static int prev_row = -1, prev_col = -1;

    int best_row = -1, best_col = -1, best_val = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int d = abs(read_hall_raw(row, col) - baseline[row][col]);
            if (d > best_val) {
                best_val = d;
                best_row = row;
                best_col = col;
            }
        }
    }

    if (best_val < HALL_DEAD_ZONE) {
        if (prev_row != -1) Serial.println("Aucun aimant détecté.");
        prev_row = prev_col = -1;
        return;
    }

    // on ne confirme que si la même case sort deux scans de suite
    if (best_row == prev_row && best_col == prev_col) {
        Serial.printf("Aimant en %c%d  (deviation=%d)\n", 'A' + best_row, best_col + 1, best_val);
    }

    prev_row = best_row;
    prev_col = best_col;
}

static void test_decoder() {
    Serial.println("=== Test décodeur ligne ===");
    Serial.println("Pose l'aimant sur la colonne 1, puis note quelle ligne répond.");
    Serial.println();
    select_col(0);  // fix column mux to col 0
    delayMicroseconds(200);
    for (int row = 0; row < 8; row++) {
        select_row(row);
        delayMicroseconds(500);
        int val = analogRead(HALL_DOUT);
        Serial.printf("Ligne %c (A0=%d A1=%d A2=%d) → ADC=%d\n",
            'A' + row,
            (row >> 0) & 1,
            (row >> 1) & 1,
            (row >> 2) & 1,
            val);
    }
    Serial.println();
    delay(1000);
}

void loop() {
    test_decoder();
}
