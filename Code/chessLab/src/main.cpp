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

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

static int baseline[8][8];

static int xy_to_index(int row, int col) {
    return row * 8 + col;
}

static int read_hall_raw(int row, int col) {
    int r = row;  // sans inversion
    digitalWrite(DEC_A0, (r >> 0) & 1);
    digitalWrite(DEC_A1, (r >> 1) & 1);
    digitalWrite(DEC_A2, (r >> 2) & 1);
    digitalWrite(SEL_S0, (col >> 0) & 1);
    digitalWrite(SEL_S1, (col >> 1) & 1);
    digitalWrite(SEL_S2, (col >> 2) & 1);
    delayMicroseconds(200);
    analogRead(HALL_DOUT);
    delayMicroseconds(50);
    return analogRead(HALL_DOUT);
}

void setup() {
    Serial.begin(115200);
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

    Serial.println("Calibration plateau vide...");
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            baseline[row][col] = read_hall_raw(row, col);
    Serial.println("=== ChessLab — SS49E scan ===");
}

static void dump_grid() {
    Serial.println("    1    2    3    4    5    6    7    8");
    for (int row = 0; row < 8; row++) {
        char letter = 'A' + row;
        Serial.printf("%c  ", letter);
        for (int col = 0; col < 8; col++) {
            int val = read_hall_raw(row, col);
            Serial.printf("%4d ", val - baseline[row][col]);
        }
        Serial.println();
    }
    Serial.println();
}

void loop() {
    static uint32_t last_dump = 0;
    if (millis() - last_dump > 2000) {
        dump_grid();
        last_dump = millis();
    }

    int dev[8][8];
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++) {
            int val = read_hall_raw(row, col);
            int d   = abs(val - baseline[row][col]);
            dev[row][col] = (d < HALL_DEAD_ZONE) ? 0 : d;
        }

    strip.clear();
    bool any = false;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (dev[row][col] == 0) continue;
            int d = dev[row][col];
            if (row > 0 && dev[row-1][col] > d) continue;
            if (row < 7 && dev[row+1][col] > d) continue;
            if (col > 0 && dev[row][col-1] > d) continue;
            if (col < 7 && dev[row][col+1] > d) continue;
            if (!any) { Serial.print("Active: "); any = true; }
            Serial.printf("%c%d(%d) ", 'A' + row, col + 1, d);
            int brightness = map(constrain(d, 0, HALL_RANGE), 0, HALL_RANGE, 0, 255);
            strip.setPixelColor(xy_to_index(row, col), strip.Color(brightness, 0, 0));
        }
    }
    if (any) Serial.println();
    strip.show();
    delay(100);
}