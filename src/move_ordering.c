//
// Created by Logan on 9/26/2024.
//
#include "types.h"

static uint8_t get_flags(uint16_t move) {
    return move >> 12;
}

// checks if flags match double pawn push
static uint8_t is_double_pawn_push(uint8_t flags) {
    return flags == 0x1;
}

// checks if flags match en passant capture
static uint8_t is_en_passant(uint8_t flags) {
    return flags == 0x5;
}

// checks if flags match castle
static uint8_t is_castle(uint8_t flags) {
    return flags == 0x2 || flags == 0x3;
}

// checks if flags match capture
static uint8_t is_capture(uint8_t flags) {
    return (flags & 0x4) >> 2;
}

// checks if flags match promotion
static uint8_t is_promotion(uint8_t flags) {
    return (flags & 0x8) >> 3;
}

static uint8_t get_from_sq(uint16_t move) {
    return move & 0x3F;
}

// extracts the 6 bits of the move corresponding to the "to square" of the move
static uint8_t get_to_sq(uint16_t move) {
    return (move >> 6) & 0x3F;
}

uint16_t get_move_score(board *bd, game_state *st, uint16_t move) {
    static uint16_t to_score[12] = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6};
    static uint16_t from_score[12] = {5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0};
    if (is_en_passant(get_flags(move))) {
        return 105;
    }
    if (is_capture(get_flags(move))) {
        return 100 * to_score[bd->type_arr[get_to_sq(move)]] + from_score[bd->type_arr[get_from_sq(move)]];
    }
    return 0;
}

void score_moves(board *bd, game_state *st, uint16_t *move_list, uint16_t *score_list, uint32_t num_moves) {
    for (uint32_t i = 0; i < num_moves; i++) {
        score_list[i] = get_move_score(bd, st, move_list[i]);
    }
}

#include "move_ordering.h"
