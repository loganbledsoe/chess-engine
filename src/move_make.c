#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "move.h"
#include "print.h"
//
// Created by Logan on 9/10/2024.
//

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
 * 0x4     	0 	        1 	        0 	        0 	        capture
 * 0x5     	0 	        1         	0 	        1 	        ep-capture
 * 0x8 	    1 	        0         	0 	        0 	        knight-promotion
 * 0x9 	    1 	        0         	0 	        1 	        bishop-promotion
 * 0xa 	    1 	        0         	1 	        0 	        rook-promotion
 * 0xb 	    1         	0         	1         	1 	        queen-promotion
 * 0xc 	    1         	1         	0         	0         	knight-promo capture
 * 0xd 	    1 	        1         	0         	1         	bishop-promo capture
 * 0xe 	    1 	        1         	1         	0 	        rook-promo capture
 * 0xf 	    1 	        1 	        1 	        1 	        queen-promo capture
 *
 * Flag encoding based on: https://www.chessprogramming.org/Encoding_Moves
 *
 */

extern start_sq st_sq;
extern castling_info castl_info[TEAM_NB][CASTLING_SIDES];

// checks if flags match double pawn push
static uint8_t is_double_pawn_push(uint8_t flags) {
    return flags == 0x1;
}

// checks if flags match en passant capture
static uint8_t is_en_passant(uint8_t flags) {
    return flags == 0x5;
}

// checks if flags match king caslte
static uint8_t is_king_castle(uint8_t flags) {
    return flags == 0x2;
}

// checks if flags match queen castle
static uint8_t is_queen_castle(uint8_t flags) {
    return flags == 0x3;
}

// returns nonzero value if capture flag is set
static uint8_t is_capture(uint8_t flags) {
    return flags & 0x4;
}

// returns nonzero value if promotion flag is set
static uint8_t is_promotion(uint8_t flags) {
    return flags & 0x8;
}

// !TODO: refactor
static void update_castling_rights(game_state *state, uint8_t from_sq, uint8_t to_sq) {
    if (from_sq == st_sq.w_king) {
        if (state->castling_rights[WHITE][KING_SIDE]) {
            state->castling_rights[WHITE][KING_SIDE] = FALSE;
            state->position_hash ^= castling_hash[WHITE][KING_SIDE];
        }
        if (state->castling_rights[WHITE][QUEEN_SIDE]) {
            state->castling_rights[WHITE][QUEEN_SIDE] = FALSE;
            state->position_hash ^= castling_hash[WHITE][QUEEN_SIDE];
        }
    }
    if (from_sq == st_sq.b_king) {
        if (state->castling_rights[BLACK][KING_SIDE]) {
            state->castling_rights[BLACK][KING_SIDE] = FALSE;
            state->position_hash ^= castling_hash[BLACK][KING_SIDE];
        }
        if (state->castling_rights[BLACK][QUEEN_SIDE]) {
            state->castling_rights[BLACK][QUEEN_SIDE] = FALSE;
            state->position_hash ^= castling_hash[BLACK][QUEEN_SIDE];
        }
    }
    if (from_sq == st_sq.w_k_rook || to_sq == st_sq.w_k_rook) {
        if (state->castling_rights[WHITE][KING_SIDE]) {
            state->castling_rights[WHITE][KING_SIDE] = FALSE;
            state->position_hash ^= castling_hash[WHITE][KING_SIDE];
        }
    }
    if (from_sq == st_sq.w_q_rook || to_sq == st_sq.w_q_rook) {
        if (state->castling_rights[WHITE][QUEEN_SIDE]) {
            state->castling_rights[WHITE][QUEEN_SIDE] = FALSE;
            state->position_hash ^= castling_hash[WHITE][QUEEN_SIDE];
        }
    }
    if (from_sq == st_sq.b_k_rook || to_sq == st_sq.b_k_rook) {
        if (state->castling_rights[BLACK][KING_SIDE]) {
            state->castling_rights[BLACK][KING_SIDE] = FALSE;
            state->position_hash ^= castling_hash[BLACK][KING_SIDE];
        }
    }
    if (from_sq == st_sq.b_q_rook || to_sq == st_sq.b_q_rook) {
        if (state->castling_rights[BLACK][QUEEN_SIDE]) {
            state->castling_rights[BLACK][QUEEN_SIDE] = FALSE;
            state->position_hash ^= castling_hash[BLACK][QUEEN_SIDE];
        }
    }
}

static void move_piece(board *bd, game_state *st, const uint8_t from_sq,
                       const uint8_t to_sq) {
    const uint8_t type = bd->type_arr[from_sq];
    const uint8_t team = type % 2;

    bd->type_arr[from_sq] = NONE_TYPE;
    bd->type_bb[type] ^= 0x1ULL << from_sq;
    bd->team_bb[team] ^= 0x1ULL << from_sq;
    st->position_hash ^= type_sq_hash[type][from_sq];

    bd->type_arr[to_sq] = type;
    bd->type_bb[type] |= 0x1ULL << to_sq;
    bd->team_bb[team] |= 0x1ULL << to_sq;
    st->position_hash ^= type_sq_hash[type][to_sq];

    bd->occupancy_bb = bd->team_bb[WHITE] | bd->team_bb[BLACK];
}

static uint8_t handle_castling(board *bd, game_state *st, uint8_t flags) {
    if (flags == KING_CASTLE_FLAGS || flags == QUEEN_CASTLE_FLAGS) {
        castling_info ci = castl_info[st->team_to_move][flags - KING_CASTLE_FLAGS];
        move_piece(bd, st, ci.king_start, ci.king_end);
        move_piece(bd, st, ci.rook_start, ci.rook_end);
        return TRUE;
    }
    return FALSE;
}

static uint8_t undo_castling(board *bd, game_state *st, uint8_t flags) {
    if (flags == KING_CASTLE_FLAGS || flags == QUEEN_CASTLE_FLAGS) {
        castling_info ci = castl_info[!st->team_to_move][flags - KING_CASTLE_FLAGS];
        move_piece(bd, st, ci.king_end, ci.king_start);
        move_piece(bd, st, ci.rook_end, ci.rook_start);
        return TRUE;
    }
    return FALSE;
}

// Makes the move described by the passed encoded move
// Undefined behavior for illegal or ill encoded moves
void make_move(board *bd, game_state *state, const uint16_t move) {
    state->last_move = move;

    // Get flags, from, and to squares
    const uint8_t flags = get_flags(move);
    const uint8_t from_sq = get_from_sq(move);
    // Note that to_sq is later modified in the case of an en passant capture
    uint8_t to_sq = get_to_sq(move);

    update_castling_rights(state, from_sq, to_sq);
    if (handle_castling(bd, state, flags)) {
        state->half_move_cnt = 0;
        state->castling_rights[state->team_to_move][KING_SIDE] = FALSE;
        state->castling_rights[state->team_to_move][QUEEN_SIDE] = FALSE;
        state->team_to_move = !state->team_to_move;
        return;
    }

    // Get the moving piece's team and type
    const uint8_t piece_team = state->team_to_move;
    // Note piece_type is later modified in the case of a promotion
    int8_t piece_type = bd->type_arr[from_sq];

    // Get captured piece type (may be NONE_TYPE)
    // Note that this may be modified in the case of an en passant capture
    int8_t captured_type = bd->type_arr[to_sq];

    // Increment half move counter
    // Note this is done before any checks to clear the counter
    state->half_move_cnt += 1;

    // Because a pawn move is irreversible, reset half move counter if one occurs
    if (piece_type == W_PAWN || piece_type == B_PAWN) {
        state->half_move_cnt = 0;
    }

    // Clear the "from square" in array and bitboard representation
    bd->type_arr[from_sq] = NONE_TYPE;
    bd->team_bb[piece_team] ^= 0x1ULL << from_sq;
    bd->type_bb[piece_type] ^= 0x1ULL << from_sq;
    state->position_hash ^= type_sq_hash[piece_type][from_sq];

    // Special flags 0 and 1 together define the promotion type
    // Define lookup tables that maps them to piece types
    // static int8_t pro_table_w[4] = {W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN};
    // static int8_t pro_table_b[4] = {B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN};
    // In the case of promotion, modify to type to the promoted type
    // Note this is done before the "to square" is set
    if (is_promotion(flags)) {
        // 0x3 masks lower two bits, special 1 and 0, of flags
        //piece_type = (piece_team == WHITE) ? pro_table_w[flags & 0x3] : pro_table_b[flags & 0x3];
        piece_type = W_KNIGHT + piece_team - 2 * (flags & 0x3);
    }

    // Set "to square" in array and bitboard representation
    bd->type_arr[to_sq] = piece_type;
    bd->team_bb[piece_team] |= 0x1ULL << to_sq;
    bd->type_bb[piece_type] |= 0x1ULL << to_sq;
    state->position_hash ^= type_sq_hash[piece_type][to_sq];

    /*
     * In the case of an en passant capture:
     * - Set the "to square" to the location of the pawn captured en passant.
     * - Set the captured type and team to match the pawn captured en passant.
     * - Note the above two are done before the "to square" is cleared.
     *
     * - Additionally, clear the captured pawn in the array representation.
     *   The captured pawn is cleared in the bitboard representation later.
     */
    if (is_en_passant(flags)) {
        // A white pawn captures en passant 1 rank below its destination square
        // A black pawn captures en passant 1 rank above its destination square
        // -8 squares == 1 rank below, +8 squares == 1 rank above
        to_sq += 8 * (piece_team == BLACK) - 8 * (piece_team == WHITE);
        captured_type = bd->type_arr[to_sq];
        bd->type_arr[to_sq] = NONE_TYPE;
    }

    state->last_captured_type = captured_type;

    /*
     * If the move is a capture, clear the "to square" in the bitboard representations.
     * The array representations have already been updated.
     *
     * Also reset half move counter since a capture is irreversible.
     */
    if (is_capture(flags)) {
        bd->team_bb[!piece_team] ^= 0x1ULL << to_sq;
        bd->type_bb[captured_type] ^= 0x1ULL << to_sq;
        state->half_move_cnt = 0;
        state->position_hash ^= type_sq_hash[captured_type][to_sq];
    }

    // Update the occupancy bitboard
    bd->occupancy_bb = bd->team_bb[WHITE] | bd->team_bb[BLACK];

    // If the move is a double pawn push, update square where en passant is possible
    // Otherwise set en passant square to none
    state->position_hash ^= en_passant_sq_hash[state->en_passant_sq];
    if (is_double_pawn_push(flags)) {
        state->en_passant_sq = to_sq;
        state->position_hash ^= en_passant_sq_hash[to_sq];
    } else {
        state->en_passant_sq = SQ_NONE;
    }

    // Swap team to move
    state->team_to_move = !piece_team;
    state->position_hash ^= black_to_move_hash;

    state->in_check = get_threats(bd, state, __builtin_ctzll(bd->type_bb[!piece_team]), !piece_team) ? TRUE : FALSE;
}

void unmake_move(board *bd, game_state *cur_state, game_state *prev_state) {
    uint16_t move = cur_state->last_move;
    uint8_t flags = get_flags(move);

    if (undo_castling(bd, cur_state, flags)) {
        memcpy(cur_state, prev_state, sizeof(game_state));
        return;
    }

    uint8_t captured_type = cur_state->last_captured_type;
    uint8_t from_sq = get_from_sq(move);
    uint8_t to_sq = get_to_sq(move);
    uint8_t moved_type = bd->type_arr[to_sq];
    uint8_t moved_team = !cur_state->team_to_move;

    // unset to square bb
    bd->type_bb[moved_type] ^= 0x1ULL << to_sq;
    bd->team_bb[moved_team] ^= 0x1ULL << to_sq;

    // switch moved type if promotion
    if (is_promotion(flags)) {
        moved_type = W_PAWN + moved_team;
    }

    // set from square
    bd->type_arr[from_sq] = moved_type;
    bd->type_bb[moved_type] |= 0x1ULL << from_sq;
    bd->team_bb[moved_team] |= 0x1ULL << from_sq;

    // set to_sq to captured type
    if (is_en_passant(flags)) {
        bd->type_arr[to_sq] = NONE_TYPE;
        to_sq += 8 * (moved_team == BLACK) - 8 * (moved_team == WHITE);
    }

    bd->type_arr[to_sq] = captured_type;
    if (captured_type != NONE_TYPE) {
        bd->type_bb[captured_type] |= 0x1ULL << to_sq;
        bd->team_bb[!moved_team] |= 0x1ULL << to_sq;
    }

    // update occupancy
    bd->occupancy_bb = bd->team_bb[WHITE] | bd->team_bb[BLACK];

    // restore game_state
    memcpy(cur_state, prev_state, sizeof(game_state));
}
