//
// Created by Logan on 9/19/2024.
//

#include "hash.h"

uint64_t type_sq_hash[TYPE_NB][SQ_NB];
uint64_t en_passant_sq_hash[SQ_NB + 1];
uint64_t castling_hash[TEAM_NB][CASTLING_SIDES];
uint64_t black_to_move_hash;

// returns a pseudo random unsigned 64-bit integer
static uint64_t xorshift64star(void) {
    static uint64_t x = 0x2C1929944248EEULL; // seed
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

// initializes pseudo random numbers for calculating position hashes
void init_hash(void) {
    // piece type square hashes
    for (uint32_t type = 0; type < TYPE_NB; ++type) {
        for (int sq = 0; sq < SQ_NB; ++sq) {
            type_sq_hash[type][sq] = xorshift64star();
        }
    }

    // en passant column hashes
    uint64_t en_passant_col_hash[COL_NB];
    for (uint32_t col = 0; col < COL_NB; ++col) {
        en_passant_col_hash[col] = xorshift64star();
    }
    // place values in array for sq lookup for convince
    for (uint32_t sq = 0; sq < SQ_NB; ++sq) {
        en_passant_sq_hash[sq] = en_passant_col_hash[sq % COL_NB];
    }
    en_passant_sq_hash[SQ_NONE] = 0;

    // castling hashes
    for (uint32_t i = 0; i < TEAM_NB; ++i) {
        for (uint32_t j = 0; j < CASTLING_SIDES; ++j) {
            castling_hash[i][j] = xorshift64star();
        }
    }

    // black to move hash
    black_to_move_hash = xorshift64star();
}

// calculates position hash from scratch for use after creating board.
// otherwise it is preferred to update hash incrementally
uint64_t calculate_hash(board *bd, game_state *st) {
    uint64_t hash = 0x0ULL;

    for (uint32_t sq = 0; sq < SQ_NB; ++sq) {
        hash ^= type_sq_hash[bd->type_arr[sq]][sq];
    }

    for (uint32_t team = 0; team < TEAM_NB; ++team) {
        for (uint32_t side = 0; side < CASTLING_SIDES; ++side) {
            hash ^= castling_hash[team][side] * st->castling_rights[team][side];
        }
    }

    hash ^= en_passant_sq_hash[st->en_passant_sq];

    hash ^= black_to_move_hash * st->team_to_move;

    return hash;
}