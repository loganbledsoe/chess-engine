//
// Created by Logan on 9/10/2024.
//

#ifndef MOVE_MAKE_H
#define MOVE_MAKE_H
#include "types.h"
void make_move(board *bd, game_state *state, const uint16_t move);
void unmake_move(board *bd, game_state *cur_state, game_state *prev_state);
#endif //MOVE_MAKE_H
