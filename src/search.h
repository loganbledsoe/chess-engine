//
// Created by Logan on 9/26/2024.
//

#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

uint16_t search(board *bd, game_state *st, uint32_t depth);
int32_t alpha_beta(board *bd, game_state *st, int32_t alpha, int32_t beta, int32_t depth);
#endif //SEARCH_H
