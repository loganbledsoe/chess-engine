//
// Created by Logan on 9/9/2024.
//

#ifndef MOVE_GEN_H
#define MOVE_GEN_H
#include "types.h"

uint64_t get_moves(board *bd, game_state *state, const uint8_t sq);
uint32_t get_move_list(board *bd, game_state * state, uint16_t *moves_list);
uint64_t get_threats(board *board, game_state * const state, const uint8_t sq, uint8_t team);
void init_masks();
void init_castling_info(board *bd);
uint8_t is_range_safe(board *bd, game_state *state, uint8_t low_sq, uint8_t high_sq);
uint8_t is_range_empty(board *bd, game_state *state, uint8_t low_sq, uint8_t high_sq);
uint32_t remove_illegal_moves(board *bd, game_state *state, uint16_t *moves_list, uint32_t num_moves);

#endif //MOVE_GEN_H
