#include <Arduino.h>
#include "board_config.h"
#include "hal_sensors.h"

// Écrit une valeur 3 bits sur trois broches de sortie (bit0 → p0, bit1 → p1, bit2 → p2)
static void write_addr(int p0, int p1, int p2, int val) {
    digitalWrite(p0,  val       & 1);
    digitalWrite(p1, (val >> 1) & 1);
    digitalWrite(p2, (val >> 2) & 1);
}

void sensors_init() {
    pinMode(HALL_DOUT, INPUT);
    for (int p : {DEC_A0, DEC_A1, DEC_A2, SEL_S0, SEL_S1, SEL_S2})
        pinMode(p, OUTPUT);
}

void sensors_select_row(int row) {
    digitalWrite(DEC_A0, (row >> 0) & 1);
    digitalWrite(DEC_A1, (row >> 1) & 1);
    digitalWrite(DEC_A2, (row >> 2) & 1);
}

void sensors_select_col(int col) {
    digitalWrite(SEL_S2, (col >> 2) & 1);
    digitalWrite(SEL_S1, (col >> 1) & 1);
    digitalWrite(SEL_S0, (col >> 0) & 1);
}

int sensors_read_raw(int row, int col) {
    sensors_select_col(col);
    sensors_select_row(row);
    delayMicroseconds(2000);  // stabilisation mux

    // 30 échantillons espacés de 60 µs : les pics WiFi n'affectent que quelques samples
    // Moyenne tronquée (jeter 6 extremes) → robuste aux rafales WiFi
    const int N = 30, TRIM = 6;
    int s[N];
    for (int i = 0; i < N; i++) {
        s[i] = analogRead(HALL_DOUT);
        delayMicroseconds(60);
    }

    for (int i = 1; i < N; i++) {
        int key = s[i], j = i - 1;
        while (j >= 0 && s[j] > key) { s[j+1] = s[j]; j--; }
        s[j+1] = key;
    }

    int sum = 0;
    for (int i = TRIM; i < N - TRIM; i++) sum += s[i];
    return sum / (N - 2 * TRIM);
}
