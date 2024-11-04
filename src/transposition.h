//
// Created by Logan on 9/27/2024.
//

#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "types.h"

typedef struct {
    uint16_t key; // most significant 16 bits of position hash
    uint16_t move; // best move
    int16_t score; // position evaluation
    uint8_t type; // type of evaluation: exact, lower bound, or upper bound
    uint8_t depth; // depth of search, 0 for quiescence search (planned)

} transposition;

enum score_type {
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
};

void record_transposition(uint64_t hash, uint16_t move, int16_t score, uint8_t type, uint8_t depth);
transposition* lookup_transposition(uint64_t hash);

#endif //TRANSPOSITION_H
