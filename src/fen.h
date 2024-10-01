//
// Created by Logan on 9/15/2024.
//

#ifndef FEN_H
#define FEN_H

#include "types.h"

int parse_fen(char *str, board *board, game_state *state);
void create_bb_from_arr(board *board);

#endif //FEN_H
