//
// Created by Logan on 9/13/2024.
//

#ifndef PERFT_H
#define PERFT_H
#include "types.h"

typedef struct {
    uint64_t nodes;
    uint64_t captures;
    uint64_t en_passant;
    uint64_t castles;
    uint64_t promotions;
    uint64_t checks;
    uint64_t checkmates;
} perft_results;

uint64_t perft(board *bd, game_state *state, uint32_t depth);
void perft_handler(board *bd, game_state *state, uint32_t depth);
void perft_detailed(perft_results *results, board *bd, game_state *state, uint32_t depth);
void perft_handler_details(board *bd, game_state *state, uint32_t depth);

#endif //PERFT_H
