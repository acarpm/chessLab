#include "board_config.h"
#include "chess_game.h"

static BoardState board;

void game_init() {
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            board.squares[r][c] = SQUARE_EMPTY;
}

void game_update(int dev[BOARD_SIZE][BOARD_SIZE]) {
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            board.squares[r][c] = (dev[r][c] >= THRESHOLD) ? SQUARE_OCCUPIED : SQUARE_EMPTY;
}

BoardState* game_get_state() {
    return &board;
}
