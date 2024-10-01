//
// Created by Logan on 9/19/2024.
//

#ifndef HASH_H
#define HASH_H
#include "types.h"

extern uint64_t type_sq_hash[TYPE_NB][SQ_NB];
extern uint64_t en_passant_sq_hash[SQ_NB + 1];
extern uint64_t castling_hash[TEAM_NB][CASTLING_SIDES];
extern uint64_t black_to_move_hash;

void init_hash(void);
uint64_t calculate_hash(board *bd, game_state *st);

#endif //HASH_H
