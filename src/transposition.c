//
// Created by Logan on 9/27/2024.
//

#include "transposition.h"

#define TT_SIZE 1000007

transposition transposition_table[TT_SIZE];

void record_transposition(uint64_t hash, uint16_t move,
                          int16_t score, uint8_t type, uint8_t depth) {
    const uint32_t index = hash % TT_SIZE;

    transposition_table[index].key = hash >> 48;
    transposition_table[index].move = move;
    transposition_table[index].score = score;
    transposition_table[index].type = type;
    transposition_table[index].depth = depth;
}

transposition* lookup_transposition(uint64_t hash) {

    const uint32_t index = hash % TT_SIZE;
    if (transposition_table[index].key == (hash >> 48)) {
        return &transposition_table[index];
    }
    return NULL;
}