//
// Created by Logan on 9/13/2024.
//

#ifndef PRINT_H
#define PRINT_H
#include <stdio.h>

#include "move.h"
#include "types.h"

void print_move_list(move_list *mv_list);
void print_board(board *bd, game_state *state);
void print_uint64(uint64_t value);
uint8_t bb_match_arr(board *bd);
void print_move(uint16_t move);

#endif //PRINT_H
