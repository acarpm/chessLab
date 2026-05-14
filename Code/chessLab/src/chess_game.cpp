#include "board_config.h"
#include "chess_game.h"

static BoardState board;
static GamePhase  phase = GAME_SETUP;

void game_init() {
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            board.squares[r][c] = SQUARE_EMPTY;
    phase = GAME_SETUP;
}

void game_update(int dev[BOARD_SIZE][BOARD_SIZE]) {
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            board.squares[r][c] = (dev[r][c] >= THRESHOLD) ? SQUARE_OCCUPIED : SQUARE_EMPTY;
}

BoardState* game_get_state()              { return &board; }
GamePhase   game_get_phase()              { return phase; }
void        game_set_phase(GamePhase p)   { phase = p; }

// Chaque colonne cible (0,1,6,7) doit avoir au moins START_MIN_PER_COL pièces.
// Colonnes 2-5 doivent être vides.
bool game_check_start_position(int dev[BOARD_SIZE][BOARD_SIZE]) {
    int cnt[8] = {};
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            if (dev[r][c] >= THRESHOLD) cnt[c]++;

    const int targets[] = {0, 1, 6, 7};
    for (int i = 0; i < 4; i++)
        if (cnt[targets[i]] < START_MIN_PER_COL) return false;

    for (int c = 2; c <= 5; c++)
        if (cnt[c] > 0) return false;

    return true;
}
