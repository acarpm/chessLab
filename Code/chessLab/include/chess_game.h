#pragma once
#include "board_config.h"

typedef enum {
    SQUARE_EMPTY = 0,
    SQUARE_OCCUPIED
} SquareState;

typedef struct {
    SquareState squares[BOARD_SIZE][BOARD_SIZE];
} BoardState;

typedef enum {
    GAME_SETUP = 0,
    GAME_PLAYING
} GamePhase;

void        game_init();
void        game_update(int dev[BOARD_SIZE][BOARD_SIZE]);
BoardState* game_get_state();
GamePhase   game_get_phase();
void        game_set_phase(GamePhase p);
bool        game_check_start_position(int dev[BOARD_SIZE][BOARD_SIZE]);
