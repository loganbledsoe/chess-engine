//
// Created by Logan on 9/15/2024.
//

#include "fen.h"

#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "move.h"
#include "types.h"

#define NUM_TOKENS 6

static uint8_t char_to_piece_type[] = {
    ['K'] = W_KING,
    ['k'] = B_KING,
    ['Q'] = W_QUEEN,
    ['q'] = B_QUEEN,
    ['R'] = W_ROOK,
    ['r'] = B_ROOK,
    ['B'] = W_BISHOP,
    ['b'] = B_BISHOP,
    ['N'] = W_KNIGHT,
    ['n'] = B_KNIGHT,
    ['P'] = W_PAWN,
    ['p'] = B_PAWN
};

static uint8_t char_to_team[] = {
    ['w'] = WHITE,
    ['b'] = BLACK
};

void create_bb_from_arr(board *board) {
    for (uint8_t i = 0; i < SQ_NB; ++i) {
        uint8_t type = board->type_arr[i];
        if (type != NONE_TYPE) {
            board->type_bb[type] |= 0x1ULL << i;
            board->team_bb[type % 2] |= 0x1ULL << i;
        }
    }
    board->occupancy_bb = board->team_bb[WHITE] | board->team_bb[BLACK];
}

static int split_str(char *str, char delim, char **dest, int dest_sz) {
    char *cur_start = str;
    int count = 0;

    while (*str != '\0') {
        if (*str == delim) {
            if (count >= dest_sz) {
                return FAILURE;
            }
            *str = '\0';
            dest[count++] = cur_start;
            cur_start = str + 1;
        }
        ++str;
    }

    if (count >= dest_sz) {
        return FAILURE;
    }
    dest[count++] = cur_start;

    return count;
}

static int is_piece(const char c) {
    static char piece_types[] = {
        'K', 'k', 'Q', 'q', 'R', 'r',
        'B', 'b', 'N', 'n', 'P', 'p'
    };

    for (int i = 0; i < TYPE_NB; ++i) {
        if (c == piece_types[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

static int is_num(const char c) {
    return c >= '0' && c <= '9';
}

static int char_to_int(const char c) {
    return c - '0';
}

static int is_rank(const char c) {
    return c >= '1' && c <= '8';
}

static int get_row(const char c) {
    return c - '1';
}

static int is_file(const char c) {
    return c >= 'a' && c <= 'h';
}

static int get_col(const char c) {
    return c - 'a';
}

static int parse_board(const char *str, board *board) {
    for (int row = 7; row >= 0; --row) {
        for (int col = 0; col < COL_NB; ++col) {
            const char cur_char = *str;
            // case where char corresponds to a piece type
            if (is_piece(cur_char)) {
                board->type_arr[row * COL_NB + col] = char_to_piece_type[cur_char];
            // case where char corresponds to empty squares
            } else if (is_num(cur_char)) {
                // calculate the column after setting the empty squares
                const int new_col = col + char_to_int(cur_char);
                // return failure if that puts the column out of bounds for the current row
                if (new_col > COL_NB) {
                    return FAILURE;
                }
                // set the empty squares
                while (col < new_col) {
                    board->type_arr[row * COL_NB + col] = NONE_TYPE;
                    ++col;
                }
                // compensate for col increment on last iteration of above while loop
                --col;
            // case where char is invalid
            } else {
                return FAILURE;
            }
            // move str ptr to next character
            ++str;
        }
        // end of row reached: expected is either a '/' to separate the row
        // or a null character to end the string
        if ((row != 0 && *str != '/') || (row == 0 && *str != '\0')) {
            return FAILURE;
        }
        // move str ptr to next character to skip '/'
        ++str;
    }
    create_bb_from_arr(board);
    return SUCCESS;
}

static int parse_team_to_move(const char *str, game_state *state) {
    if (*str == 'w') {
        state->team_to_move = WHITE;
    } else if (*str == 'b') {
        state->team_to_move = BLACK;
    } else {
        return FAILURE;
    }

    if (*(str + 1) != '\0') {
        return FAILURE;
    }

    return SUCCESS;
}

static int parse_en_passant(const char *str, game_state *state) {
    if (*str == '-') {
        state->en_passant_sq = SQ_NONE;
        if (*(str + 1) != '\0') {
            return FAILURE;
        }
    } else if (is_file(*str) && is_rank(*(str + 1))) {
        state->en_passant_sq = get_row(*(str + 1)) * COL_NB + get_col(*str);
        if (*(str + 2) != '\0') {
            return FAILURE;
        }
    } else {
        return FAILURE;
    }
    return SUCCESS;
}

static int parse_half_move_cnt(const char *str, game_state *state) {
    if (is_num(*str)) {
        state->half_move_cnt = char_to_int(*str);
        if (is_num(*(str + 1))) {
            state->half_move_cnt *= 10;
            state->half_move_cnt += char_to_int(*(str + 1));
            if (*(str + 2) != '\0') {
                return FAILURE;
            }
        } else if (*(str + 1) != '\0') {
            return FAILURE;
        }
    } else {
        return FAILURE;
    }
    return SUCCESS;
}

static int parse_castling_rights(char *str, game_state *state) {
    static char castling_symbols[] = {'K', 'Q', 'k', 'q'};

    if (*str == '-') {
        if (*(str + 1) != '\0') {
            return FAILURE;
        }
        return SUCCESS;
    }
    if (*str == '\0') {
        return FAILURE;
    }

    for (int i = 0; i < 4; ++i) {
        if (*str == castling_symbols[i]) {
            state->castling_rights[i / 2][i % 2] = TRUE;
            ++str;
        } else if (*str == '\0') {
            return SUCCESS;
        }
    }
    if (*str != '\0') {
        return FAILURE;
    }
    return SUCCESS;
}

int parse_fen(char *str, board *board, game_state *state) {
    char *token[NUM_TOKENS];
    int tokens_result = split_str(str, ' ', token, NUM_TOKENS);
    if (tokens_result != NUM_TOKENS) {
        return FAILURE;
    }

    if (parse_board(token[0], board)) {
        return FAILURE;
    }
    if (parse_team_to_move(token[1], state)) {
        return FAILURE;
    }
    if (parse_castling_rights(token[2], state)) {
        return FAILURE;
    }
    if (parse_en_passant(token[3], state)) {
        return FAILURE;
    }
    if (parse_half_move_cnt(token[4], state)) {
        return FAILURE;
    }

    state->in_check = get_threats(board, state, __builtin_ctzll(board->type_bb[state->team_to_move]), state->team_to_move) ? TRUE : FALSE;

    state->position_hash = calculate_hash(board, state);
    return SUCCESS;
}
