//
// Created by Logan on 9/13/2024.
//

#include "print.h"

#include "move.h"

char sq_to_str[SQ_NB][3] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

static char type_to_str[TYPE_NB][13] = {
    [W_KING] = "WHITE KING",
    [B_KING] = "BLACK KING",
    [W_QUEEN] = "WHITE QUEEN",
    [B_QUEEN] = "BLACK QUEEN",
    [W_ROOK] = "WHITE ROOK",
    [B_ROOK] = "BLACK ROOK",
    [W_BISHOP] = "WHITE BISHOP",
    [B_BISHOP] = "BLACK BISHOP",
    [W_KNIGHT] = "WHITE KNIGHT",
    [B_KNIGHT] = "BLACK KNIGHT",
    [W_PAWN] = "WHITE PAWN",
    [B_PAWN] = "BLACK PAWN"
};

void print_uint64(uint64_t value) {

    for (int8_t i = 7; i >= 0; --i) {
        for (int8_t j = 0; j < 8; ++j) {
            int8_t pos = 8 * i + j;
            printf("%d ", (value >> pos) & 0x1ULL);
        }
        printf("\n");
    }
    printf("\n");
}

void print_move(uint16_t move) {
    printf("%s to %s, ", sq_to_str[move & 0x3f], sq_to_str[(move >> 6) & 0x3f]);
}

void print_move_list(move_list *mv_list) {
    for (uint32_t i = 0; i < mv_list->size; i++) {
        print_move(mv_list->moves[i]);
    }
    printf("\n");
}

uint8_t bb_match_arr(board *bd) {
    for (uint8_t sq = 0; sq < SQ_NB; sq++) {
        uint8_t correct_type = bd->type_arr[sq];
        for (uint8_t type = 0; type < TYPE_NB; type++) {
            if ((correct_type == type) && !((bd->type_bb[type] >> sq) & 0x1)) {
                printf("ERRORRRR: %d", type);
                return FALSE;
            }
            if ((correct_type != type) && ((bd->type_bb[type] >> sq) & 0x1)){
                printf("ERRORRRR: %d", type);
                return FALSE;
            }
        }
    }
    uint64_t white_bb = 0x0ULL;
    uint64_t black_bb = 0x0ULL;
    for (uint8_t type = 0; type < TYPE_NB; type++) {
        if (type % 2 == 0) {
            white_bb |= bd->type_bb[type];
        } else {
            black_bb |= bd->type_bb[type];
        }
    }
    if ((white_bb != bd->team_bb[WHITE])) {
        printf("TEAM BB ERROR WHITE\n");
        return FALSE;
    }
    if ((black_bb != bd->team_bb[BLACK])) {
        printf("TEAM BB ERROR BLACK\n");
        return FALSE;
    }

    if ((white_bb | black_bb) != bd->occupancy_bb) {
        printf("OCCUPANCY ERROR\n");
        return FALSE;
    }

    return TRUE;
}

void print_board(board *bd, game_state *state) {
    static char piece_unicode[] = {
        ' ',
        'K', 'k', 'Q', 'q', 'R', 'r',
        'B', 'b', 'N', 'n', 'P', 'p'
    };

    for (int8_t i = 7; i >= 0; --i) {
        printf("%d|", i + 1);
        for (int8_t j = 0; j < 8; ++j) {
            int8_t pos = 8 * i + j;
            printf("%c|", piece_unicode[bd->type_arr[pos] + 1]);
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n\n");
    if (state->en_passant_sq == SQ_NONE) {
        printf("En Passant Sq:          %s\n", "none");
    } else {
        printf("En Passant Sq:          %d\n", state->en_passant_sq);
    }
    printf("Half Move Clock:        %d\n", state->half_move_cnt);
    printf("Team to Move:           %s\n", state->team_to_move == WHITE ? "white" : "black");
    printf("White Castle Queenside: %s\n", state->castling_rights[WHITE][QUEEN_SIDE] ? "true" : "false");
    printf("White Castle Kingside:  %s\n", state->castling_rights[WHITE][KING_SIDE] ? "true" : "false");
    printf("Black Castle Queenside: %s\n", state->castling_rights[BLACK][QUEEN_SIDE] ? "true" : "false");
    printf("Black Castle Kingside:  %s\n\n", state->castling_rights[BLACK][KING_SIDE] ? "true" : "false");
    printf("Previous move:          %d to %d, flags: %d\n", state->last_move & 0x3f, (state->last_move >> 6) & 0x3f, state->last_move >> 12);
    printf("Previous captured type: %d\n", state->last_captured_type);
    printf("Matches: %s\n\n", bb_match_arr(bd) == FALSE ? "false" : "true");

    if (!bb_match_arr(bd)) {
        for (uint8_t i = 0; i < TYPE_NB; i++) {
            printf("%s:\n", type_to_str[i]);
            print_uint64(bd->type_bb[i]);
        }
        printf("WHITE:\n");
        print_uint64(bd->team_bb[WHITE]);
        printf("BLACK:\n");
        print_uint64(bd->team_bb[BLACK]);
        printf("OCCUPANCY:\n");
        print_uint64(bd->occupancy_bb);
        printf("\n");
    }
}