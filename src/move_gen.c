#include <stdio.h>
#include "move_gen.h"

#include <stdlib.h>
#include <string.h>

#include "move_make.h"
#include "print.h"

//
// Created by Logan on 9/8/2024.
//

// masks used for initializing all rank, file, diagonal, and anti-diagonal masks
#define ROW_0 0x00000000000000ffULL // first rank mask
#define FILE_0 0x0101010101010101ULL // first file mask
#define MAIN_DIAGONAL 0x8040201008040201ULL
#define MAIN_ANTI_DIAGONAL 0x0102040810204080ULL

uint64_t ROW[8]; // rank masks
uint64_t COL[8]; // file masks
uint64_t DIAG[15]; // diagonal masks
uint64_t ADIAG[15]; // anti-diagonal masks

mask msk[SQ_NB];

castling_info castl_info[TEAM_NB][CASTLING_SIDES];

// initializes castling info, the starting board should be passed
void init_castling_info(board *bd) {
    // find start squares for kings and rooks based on current board
    uint8_t w_king = __builtin_ctzll(bd->type_bb[W_KING]);
    uint8_t b_king = __builtin_ctzll(bd->type_bb[B_KING]);
    uint8_t w_q_rook = __builtin_ctzll(bd->type_bb[W_ROOK]);
    uint8_t w_k_rook = __builtin_ctzll(bd->type_bb[W_ROOK] ^ (0x1ULL << w_q_rook));
    uint8_t b_q_rook = __builtin_ctzll(bd->type_bb[B_ROOK]);
    uint8_t b_k_rook = __builtin_ctzll(bd->type_bb[B_ROOK] ^ (0x1ULL << b_q_rook));

    printf("white king start: %d\n", w_king);
    printf("black king start: %d\n", b_king);
    printf("white rook queen side start: %d\n", w_q_rook);
    printf("white rook king side start: %d\n", w_k_rook);
    printf("black rook queen side start: %d\n", b_q_rook);
    printf("black rook king side start: %d\n", b_k_rook);

    castl_info[WHITE][KING_SIDE] = (castling_info){
        .king_start = w_king,
        .rook_start = w_k_rook,
        .safe_start = w_king + 1,
        .vacant_start = w_king + 1,
        .king_end = W_K_KING_END_SQ,
        .rook_end = W_K_ROOK_END_SQ,
        .safe_end = W_K_KING_END_SQ,
        .vacant_end = w_k_rook - 1,
        .move = KING_CASTLE_MOVE,
    };

    castl_info[WHITE][QUEEN_SIDE] = (castling_info){
        .king_start = w_king,
        .rook_start = w_q_rook,
        .safe_start = W_Q_KING_END_SQ,
        .vacant_start = w_q_rook + 1,
        .king_end = W_Q_KING_END_SQ,
        .rook_end = W_Q_ROOK_END_SQ,
        .safe_end = w_king - 1,
        .vacant_end = w_king - 1,
        .move = QUEEN_CASTLE_MOVE
    };

    castl_info[BLACK][KING_SIDE] = (castling_info){
        .king_start = b_king,
        .rook_start = b_k_rook,
        .safe_start = b_king + 1,
        .vacant_start = b_king + 1,
        .king_end = B_K_KING_END_SQ,
        .rook_end = B_K_ROOK_END_SQ,
        .safe_end = B_K_KING_END_SQ,
        .vacant_end = b_k_rook - 1,
        .move = KING_CASTLE_MOVE
    };

    castl_info[BLACK][QUEEN_SIDE] = (castling_info){
        .king_start = b_king,
        .rook_start = b_q_rook,
        .safe_start = B_Q_KING_END_SQ,
        .vacant_start = b_q_rook + 1,
        .king_end = B_Q_KING_END_SQ,
        .rook_end = B_Q_ROOK_END_SQ,
        .safe_end = b_king - 1,
        .vacant_end = b_king - 1,
        .move = QUEEN_CASTLE_MOVE
    };
}

extern start_sq st_sq;

void init_masks() {
    // initialize rank and file masks
    for (int32_t i = 0; i < 8; ++i) {
        ROW[i] = ROW_0 << (8 * i);
        COL[i] = FILE_0 << i;
    }

    // initialize diagonal and anti diagonal masks
    DIAG[7] = MAIN_DIAGONAL;
    ADIAG[7] = MAIN_ANTI_DIAGONAL;
    for (int32_t i = 6; i >= 0; --i) {
        DIAG[i] = (DIAG[i + 1] >> 1) & ~COL[7];
        ADIAG[i] = (ADIAG[i + 1] >> 1) & ~COL[7];
    }
    for (int32_t i = 8; i < 15; ++i) {
        DIAG[i] = (DIAG[i - 1] << 1) & ~COL[0];
        ADIAG[i] = (ADIAG[i - 1] << 1) & ~COL[0];
    }

    // initialize per square masks
    for (int32_t sq = 0; sq < SQ_NB; ++sq) {
        msk[sq].sq = 0x1ULL << sq;
        msk[sq].row = ROW[sq / 8];
        msk[sq].col = COL[sq % 8];
        msk[sq].diag = DIAG[7 - (sq / 8) + (sq % 8)];
        msk[sq].adiag = ADIAG[(sq / 8) + (sq % 8)];
    }

}

// x = _byteswap_uint64(x);

// reverses the bits of an unsigned 64-bit integer
static uint64_t rev_uint64(uint64_t x) {
    x = (x  >> 32) | (x << 32);
    x = ((x & 0xFFFF0000FFFF0000ULL) >> 16) | ((x & 0x0000FFFF0000FFFFULL) << 16);
    x = ((x & 0xFF00FF00FF00FF00ULL) >> 8) | ((x & 0x00FF00FF00FF00FFULL) << 8);
    x = ((x & 0xF0F0F0F0F0F0F0F0ULL) >> 4) | ((x & 0x0F0F0F0F0F0F0F0FULL) << 4);
    x = ((x & 0xCCCCCCCCCCCCCCCCULL) >> 2) | ((x & 0x3333333333333333ULL) << 2);
    x = ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1) | ((x & 0x5555555555555555ULL) << 1);

    return x;
}

uint64_t get_moves_knight(board *board, game_state *state, const uint8_t sq) {
    const uint64_t pos = 0x1ULL << sq;

    uint64_t moves_left = 0x0ULL;
    moves_left |= pos << 6;
    moves_left |= pos >> 10;
    moves_left &= ~COL[6];
    moves_left |= pos << 15;
    moves_left |= pos >> 17;
    moves_left &= ~COL[7];

    uint64_t moves_right = 0x0ULL;
    moves_right |= pos >> 6;
    moves_right |= pos << 10;
    moves_right &= ~COL[1];
    moves_right |= pos >> 15;
    moves_right |= pos << 17;
    moves_right &= ~COL[0];

    return moves_left | moves_right;
}

uint64_t get_moves_king(board *board, game_state *state, const uint8_t sq) {
    const uint64_t pos = 0x1ULL << sq;

    uint64_t moves_left = ((pos >> 1) | (pos >> 9) | (pos << 7));
    moves_left &= ~COL[7];
    uint64_t moves_right = ((pos << 1) | (pos << 9) | (pos >> 7));
    moves_right &= ~COL[0];
    return moves_left | moves_right | (pos << 8) | (pos >> 8);
}

uint64_t get_moves_rook(board *board, game_state *state, const uint8_t sq) {
    const uint64_t occ = board->occupancy_bb;
    const uint64_t pos = msk[sq].sq;

    uint64_t row = occ & msk[sq].row;
    row = (row - 2 * pos) ^ rev_uint64(rev_uint64(row) - 2 * msk[63 - sq].sq);
    row &= msk[sq].row;

    uint64_t col = occ & msk[sq].col;
    col = (col - 2 * pos) ^ rev_uint64(rev_uint64(col) - 2 * msk[63 - sq].sq);
    col &= msk[sq].col;

    return row | col;
}

uint64_t get_moves_bishop(board *board, game_state *state, const uint8_t sq) {
    uint64_t occ = board->occupancy_bb;
    const uint64_t pos = msk[sq].sq;

    uint64_t diag = occ & msk[sq].diag;
    diag = (diag - 2 * pos) ^ rev_uint64(rev_uint64(diag) - 2 * msk[63 - sq].sq);
    diag &= msk[sq].diag;

    uint64_t adiag = occ & msk[sq].adiag;
    adiag = (adiag - 2 * pos) ^ rev_uint64(rev_uint64(adiag) - 2 * msk[63 - sq].sq);
    adiag &= msk[sq].adiag;

    return diag | adiag;
}

uint64_t get_moves_queen(board *board, game_state *state, const uint8_t sq) {
    return get_moves_rook(board, state, sq) | get_moves_bishop(board, state, sq);
}

uint64_t get_moves_pawn_w(board *board, game_state *state, const uint8_t sq) {
    uint64_t pos = msk[sq].sq;
    uint64_t occ = board->occupancy_bb;

    uint64_t w_moves = 0x0ULL;

    // forward one
    w_moves |= pos << 8;
    w_moves &= ~occ;

    // forward two
    w_moves |= (w_moves & ROW[2]) << 8;
    w_moves &= ~occ;

    // capture (includes en passant)
    uint64_t en_passant_bb = msk[state->en_passant_sq + 8].sq  & (board->team_bb[BLACK] << 8);
    w_moves |= (pos << 7) & (occ | en_passant_bb) & ~COL[7];
    w_moves |= (pos << 9) & (occ | en_passant_bb) & ~COL[0];
    return w_moves;
}

uint64_t get_moves_pawn_b(board *board, game_state *state, const uint8_t sq) {
    uint64_t pos = msk[sq].sq;
    uint64_t occ = board->occupancy_bb;
    uint64_t b_moves = 0x0ULL;

    // forward one
    b_moves |= pos >> 8;
    b_moves &= ~occ;

    // forward two
    b_moves |= (b_moves & ROW[5]) >> 8;
    b_moves &= ~occ;

    // capture (includes en passant)
    uint64_t en_passant_bb = msk[state->en_passant_sq - 8].sq & (board->team_bb[WHITE] >> 8);
    b_moves |= (pos >> 7) & (occ | en_passant_bb) & ~COL[0];
    b_moves |= (pos >> 9) & (occ | en_passant_bb) & ~COL[7];
    return b_moves;
}

// not used yet --- compare performace later
uint64_t get_moves_pawn(board *board, game_state *state, const uint8_t sq) {
    const uint64_t pos = 0x1ULL << sq;
    uint64_t occ = board->occupancy_bb;
    uint8_t team = state->team_to_move;
    uint64_t en_passant_bb = 0x1ULL << state->en_passant_sq;

    uint64_t w_moves = 0x0ULL;

    // forward one
    w_moves |= pos >> 8;
    w_moves &= ~occ;

    // forward two
    w_moves |= (w_moves & ROW[2] != 0x0ULL) * (w_moves >> 8);
    w_moves &= ~occ;

    // capture (includes en passant)
    w_moves |= (pos << 7) & (occ | (en_passant_bb << 8)) & ~COL[7];
    w_moves |= (pos << 9) & (occ | (en_passant_bb << 8)) & ~COL[0];

    uint64_t b_moves = 0x0ULL;

    // forward one
    b_moves |= pos << 8;
    b_moves &= ~occ;

    // forward two
    b_moves |= (b_moves & ROW[5] != 0x0ULL) * (b_moves << 8);
    b_moves &= ~occ;

    // capture (includes en passant)
    b_moves |= (pos >> 7) & (occ | (en_passant_bb >> 8)) & ~COL[0];
    b_moves |= (pos >> 9) & (occ | (en_passant_bb >> 8)) & ~COL[7];

    return (team == WHITE) * w_moves + (team == BLACK) * b_moves;
}

uint64_t get_capt_moves_pawn_w(board *board, game_state *state, const uint8_t sq) {
    uint64_t occ = board->occupancy_bb;
    const uint64_t pos = 0x1ULL << sq;
    uint64_t moves = 0x0ULL;
    moves |= (pos << 7) & occ & ~COL[7];
    moves |= (pos << 9) & occ & ~COL[0];
    return moves;
}

uint64_t get_capt_moves_pawn_b(board *board, game_state *state, const uint8_t sq) {
    uint64_t occ = board->occupancy_bb;
    const uint64_t pos = 0x1ULL << sq;
    uint64_t moves = 0x0ULL;
    moves |= (pos >> 7) & occ & ~COL[0];
    moves |= (pos >> 9) & occ & ~COL[7];
    return moves;
}

// get threats to a particular square
uint64_t get_threats(board *board, game_state * const state, const uint8_t sq, uint8_t team) {
    uint64_t threats = 0x0ULL;
    if (team == WHITE) {
        threats |= get_moves_rook(board, state, sq) & (board->type_bb[B_QUEEN] | board->type_bb[B_ROOK]);
        threats |= get_moves_bishop(board, state, sq) & (board->type_bb[B_QUEEN] | board->type_bb[B_BISHOP]);
        threats |= get_moves_knight(board, state, sq) & board->type_bb[B_KNIGHT];
        threats |= get_moves_king(board, state, sq) & board->type_bb[B_KING];
        threats |= get_capt_moves_pawn_w(board, state, sq) & board->type_bb[B_PAWN];
    } else {
        threats |= get_moves_rook(board, state, sq) & (board->type_bb[W_QUEEN] | board->type_bb[W_ROOK]);
        threats |= get_moves_bishop(board, state, sq) & (board->type_bb[W_QUEEN] | board->type_bb[W_BISHOP]);
        threats |= get_moves_knight(board, state, sq) & board->type_bb[W_KNIGHT];
        threats |= get_moves_king(board, state, sq) & board->type_bb[W_KING];
        threats |= get_capt_moves_pawn_b(board, state, sq) & board->type_bb[W_PAWN];
    }
    return threats;
}

uint64_t get_moves(board *bd, game_state *state, const uint8_t sq) {
    static uint64_t (*func[TYPE_NB])(board*, game_state*, uint8_t) = {
        get_moves_king, get_moves_king,
        get_moves_queen, get_moves_queen,
        get_moves_rook, get_moves_rook,
        get_moves_bishop, get_moves_bishop,
        get_moves_knight, get_moves_knight,
        get_moves_pawn_w, get_moves_pawn_b
    };
    return func[bd->type_arr[sq]](bd, state, sq) & ~bd->team_bb[state->team_to_move];
}

// encodes moves from to bitboard and adds them to a move list
uint32_t append_moves(board *bd, game_state *state, uint32_t from_sq,
                      uint64_t to_bb, uint16_t *moves, uint32_t num_moves) {
    while (to_bb) {
        // get to square via finding the least significant set bit
        uint32_t to_sq = __builtin_ctzll(to_bb);
        uint16_t move = from_sq;
        move |= (to_sq << 6);

        // set capture flag in non en passant case
        if (bd->type_arr[to_sq] != NONE_TYPE) {
            move |= 0x4000;
        }

        int8_t type = bd->type_arr[from_sq];
        if ((type == W_PAWN && to_sq >= 56)
            || (type == B_PAWN && to_sq < 8)) {
            // promotion
            move |= 0x8000;
            moves[num_moves++] = move | 0x3000; // queen
            moves[num_moves++] = move | 0x2000; // rook
            moves[num_moves++] = move | 0x1000; // bishop
            // knight added and end of while loop
        } else if ((type == W_PAWN || type == B_PAWN)
                   && (from_sq - to_sq) % 2 != 0
                   && bd->type_arr[to_sq] == NONE_TYPE) {
            // en passant capture
            move |= 0x5000;
        } else if ((type == W_PAWN && to_sq - from_sq == 16)
                   || (type == B_PAWN && from_sq - to_sq == 16)) {
            // double pawn push
            move |= 0x1000;
        }

        moves[num_moves++] = move;
        to_bb &= to_bb - 1;
    }
    return num_moves;
}


uint8_t is_range_safe(board *bd, game_state *state, uint8_t low_sq, uint8_t high_sq) {
    for (uint8_t sq = low_sq; sq <= high_sq; ++sq) {
        if (get_threats(bd, state, sq, state->team_to_move)) {
            return FALSE;
        }
    }
    return TRUE;
}

uint8_t is_range_empty(board *bd, game_state *state, uint8_t low_sq, uint8_t high_sq) {
    for (uint8_t sq = low_sq; sq <= high_sq; ++sq) {
        if (bd->type_arr[sq] != NONE_TYPE) {
            return FALSE;
        }
    }
    return TRUE;
}

uint32_t append_castling_moves(board *bd, game_state *state, uint16_t *moves, uint32_t num_moves) {

    const uint8_t team = state->team_to_move;
    for (int i = 0; i < 2; ++i) {
        castling_info ci = castl_info[team][i];
        if (state->castling_rights[team][i] &&
            !state->in_check &&
            is_range_safe(bd, state, ci.safe_start, ci.safe_end) &&
            is_range_empty(bd, state, ci.vacant_start, ci.vacant_end)
        ) {
            moves[num_moves++] = ci.move;
        }
    }
    return num_moves;
}


// returns list of pseudo legal moves
uint32_t get_move_list(board *bd, game_state * state, uint16_t *moves_list) {
    uint8_t team_to_move = state->team_to_move;
    uint32_t num_moves = 0;
    for (uint8_t i = 0; i < SQ_NB; ++i) {
        if (bd->team_bb[team_to_move] & (0x1ULL << i)) {
            if (bd->type_arr[i] == NONE_TYPE) {printf("uh oh!!!!!!!!\n"); print_board(bd, state);}
            uint64_t to_bb = get_moves(bd, state, i);
            num_moves = append_moves(bd, state, i, to_bb, moves_list, num_moves);
        }
    }
    num_moves = append_castling_moves(bd, state, moves_list, num_moves);
    return num_moves;
}

// removes moves that put a king in check -- used only for debugging
// engine prefers to defer these checks as they may not be required if cutoffs are found
uint32_t remove_illegal_moves(board *bd, game_state *state, uint16_t *moves_list, uint32_t num_moves) {
    uint32_t new_idx = 0;
    for (uint32_t i = 0; i < num_moves; ++i) {
        game_state momento;
        memcpy(&momento, state, sizeof(game_state));
        make_move(bd, state, moves_list[i]);
        if (!get_threats(bd, state, __builtin_ctzll(bd->type_bb[!state->team_to_move]), !state->team_to_move)) {
            moves_list[new_idx] = moves_list[i];
            ++new_idx;
        }
        unmake_move(bd, state, &momento);
    }
    return new_idx;
}

