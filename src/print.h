//
// Created by Logan on 9/13/2024.
//

#ifndef PRINT_H
#define PRINT_H
#include <stdio.h>
#include "types.h"

void print_move_list(uint16_t *move_list, uint32_t len);
void print_board(board *bd, game_state *state);
void print_uint64(uint64_t value);
uint8_t bb_match_arr(board *bd);
void print_move(uint16_t move);

#endif //PRINT_H
