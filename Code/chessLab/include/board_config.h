#pragma once

// GPIO pins
#define LED_PIN         19
#define HALL_DOUT        3
#define SEL_S0           0
#define SEL_S1           1
#define SEL_S2           2
#define DEC_A0          18
#define DEC_A1          16
#define DEC_A2          17

// Board dimensions
#define BOARD_SIZE       8
#define NUM_LEDS        64

// LED tuning
#define BRIGHTNESS      15
// #define FADE_DECAY   0.97f  // réservé pour futur effet de fondu

// Sensor tuning
#define HALL_RANGE     500
#define HALL_DEAD_ZONE  40
#define THRESHOLD       75
#define COL_OFFSET       0

// Validation position de départ : nombre minimum de pièces par colonne cible (0,1,6,7)
// Mettre à 8 pour exiger les 32 pièces complètes en jeu réel ; 1 pour tester avec peu d'aimants
#define START_MIN_PER_COL  1
