//
// Created by Logan on 9/8/2024.
//

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0

#define SUCCESS 0
#define FAILURE -1

#define INT32_MAX 2147483647
#define INT32_MIN -2147483647


#define TYPE_NB 12
enum piece_type {
    W_KING,  // white king
    B_KING, // black king
    W_QUEEN, // white queen
    B_QUEEN, // black queen
    W_ROOK, // white rook
    B_ROOK, // black rook
    W_BISHOP, // white bishop
    B_BISHOP, // black bishop
    W_KNIGHT, // white knight
    B_KNIGHT, // black knight
    W_PAWN, // white pawn
    B_PAWN, // black pawn
    NONE_TYPE
};

#define TEAM_NB 2
enum team {
    WHITE,
    BLACK,
    NONE_TEAM
};

#define SQ_NB 64
#define SQ_NONE 64

#define COL_NB 8
#define ROW_NB 8

#define CASTLING_NB 4
#define CASTLING_SIDES 2

typedef struct {
    uint8_t w_king;
    uint8_t w_q_rook;
    uint8_t w_k_rook;
    uint8_t b_king;
    uint8_t b_q_rook;
    uint8_t b_k_rook;
} start_sq;

typedef struct {
    uint64_t type_bb[TYPE_NB];
    uint64_t team_bb[TEAM_NB];
    uint64_t occupancy_bb;

    int8_t type_arr[SQ_NB];
} board;

#define SIDE_NB 2

typedef struct {
    uint8_t en_passant_sq; // the square where en passant capture is possible
    uint8_t half_move_cnt; // number of half moves since irreversible move
    uint8_t team_to_move; // team to move: 0 for white, 1 for black
    uint8_t in_check; // flag: if the team to move is in check

    uint16_t last_move; // the last move played
    int8_t last_captured_type; // the piece type captured by the last move

    uint64_t position_hash; // the Zobrist hash of the current position


    // castling rights
    uint8_t castling_rights[TEAM_NB][SIDE_NB];
} game_state;

typedef struct {
    uint64_t sq;
    uint64_t col;
    uint64_t row;
    uint64_t diag;
    uint64_t adiag;
} mask;

typedef struct {
    uint8_t king_start;
    uint8_t rook_start;
    uint8_t safe_start;
    uint8_t vacant_start;

    uint8_t king_end;
    uint8_t rook_end;
    uint8_t safe_end;
    uint8_t vacant_end;

    uint16_t move;
} castling_info;

enum end_square {
    W_K_KING_END_SQ = 6,
    W_Q_KING_END_SQ = 2,
    B_K_KING_END_SQ = 62,
    B_Q_KING_END_SQ = 58,
    W_K_ROOK_END_SQ = 5,
    W_Q_ROOK_END_SQ = 3,
    B_K_ROOK_END_SQ = 61,
    B_Q_ROOK_END_SQ = 59
};

enum castle_flags {
    KING_CASTLE_FLAGS = 0x2,
    QUEEN_CASTLE_FLAGS = 0x3
};

enum castle_moves {
    KING_CASTLE_MOVE = 0x2 << 12,
    QUEEN_CASTLE_MOVE = 0x3 << 12
};

enum side {
    KING_SIDE,
    QUEEN_SIDE
};

#endif //BOARD_H
