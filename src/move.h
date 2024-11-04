//
// Created by Logan on 10/19/2024.
//

#ifndef MOVES_H
#define MOVES_H

#include "types.h"

/*
 * MOVE ENCODING:
 * Move encoded in a 16-bit unsigned integer
 * 0000 0000 0011 1111 - from square
 * 0000 1111 1100 0000 - to square
 * 1111 0000 0000 0000 - flags
 *
 * The from square and two square are integer values between 0 and 63 inclusive
 * representing squares on the board in increasing file then increasing rank.
 * Ex: square 10 represents the square intersecting the 3rd file and 2nd rank.
 * (rank and file referring to the chess terms indexed beginning at 1)
 *
 * FLAG ENCODING:
 * code 	promotion 	capture 	special 1 	special 0 	move type
 * 0x0 	    0 	        0 	        0 	        0         	quiet move
 * 0x1 	    0 	        0 	        0         	1         	double pawn push
 * 0x2 	    0 	        0         	1 	        0         	king castle
 * 0x3 	    0 	        0 	        1 	        1 	        queen castle
 * 0x4 	    0 	        1 	        0 	        0 	        capture
 * 0x5 	    0 	        1         	0 	        1 	        en passant capture
 * 0x8 	    1 	        0         	0 	        0 	        knight promotion
 * 0x9 	    1 	        0         	0 	        1 	        bishop promotion
 * 0xa 	    1 	        0         	1 	        0 	        rook promotion
 * 0xb 	    1         	0         	1         	1 	        queen promotion
 * 0xc 	    1         	1         	0         	0         	knight promotion capture
 * 0xd 	    1 	        1         	0         	1         	bishop promotion capture
 * 0xe 	    1 	        1         	1         	0 	        rook promotion capture
 * 0xf 	    1 	        1 	        1 	        1 	        queen promotion capture
 *
 * Flag encoding based on: https://www.chessprogramming.org/Encoding_Moves
 */
typedef uint16_t move;

 static move encode_move(uint8_t from_sq, uint8_t to_sq, uint8_t flags) {
     return from_sq | (to_sq << 6) | (flags << 12);
 }

 static uint8_t get_from_sq(move mv) {
     return mv & 0x3f;
 }

 static uint8_t get_to_sq(move mv) {
     return (mv >> 6) & 0x3f;
 }

 static uint8_t get_flags(move mv) {
     return mv >> 12;
 }

// Maximum number of possible moves from any position
#define MOVE_LIST_SIZE 218

typedef struct {
    move moves[MOVE_LIST_SIZE];
    uint16_t scores[MOVE_LIST_SIZE];
    uint32_t size;
} move_list;

uint64_t get_moves(board *bd, game_state *state, const uint8_t sq);
void get_move_list(board *bd, game_state * state, move_list *mv_list);
uint64_t get_threats(board *board, game_state * const state, const uint8_t sq, uint8_t team);
void init_masks();
void init_castling_info(board *bd);
uint8_t is_range_safe(board *bd, game_state *state, uint8_t low_sq, uint8_t high_sq);
uint8_t is_range_empty(board *bd, game_state *state, uint8_t low_sq, uint8_t high_sq);
void remove_illegal_moves(board *bd, game_state *state, move_list *mv_list);

void make_move(board *bd, game_state *state, const uint16_t move);
void unmake_move(board *bd, game_state *cur_state, game_state *prev_state);

void score_moves(board *bd, game_state *st, move_list *mv_list);

#endif //MOVES_H
