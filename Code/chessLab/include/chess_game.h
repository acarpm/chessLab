#pragma once
#include "board_config.h"

// État d'une case : vide ou occupée par une pièce
typedef enum {
    SQUARE_EMPTY = 0,
    SQUARE_OCCUPIED
} SquareState;

// État complet du plateau (64 cases)
typedef struct {
    SquareState squares[BOARD_SIZE][BOARD_SIZE];
} BoardState;

void          game_init();
void          game_update(int dev[BOARD_SIZE][BOARD_SIZE]);
BoardState*   game_get_state();
