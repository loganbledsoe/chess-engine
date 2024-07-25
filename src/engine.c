#include <limits.h>
#include "board.h"
#include <stdio.h>

i32 piece_values[] = {200, -200, 9, -9, 5, -5, 3, -3, 3, -3, 1, -1};

i32 max(i32 x, i32 y) {
    return (x >= y) ? x : y;
}

i32 min(i32 x, i32 y) {
    return (x <= y) ? x : y;
}

i32 evaluate(u64 *board, u32 *state_flags) {
    i32 eval = 0;
    for (i32 i = 0; i < 12; ++i) {
        eval += 10 * __builtin_popcountll(board[i]) * piece_values[i];
    }

    // temporary
    for (i32 i = 0; i < 64; ++i) {
        if (board[W_ALL] & (0x1ULL << i)) {
            eval += __builtin_popcountll(getLegalMoves(board, state_flags, i));
        } else if (board[B_ALL] & (0x1ULL << i)) {
            eval -= __builtin_popcountll(getLegalMoves(board, state_flags, i));
        }
    }
    return eval;
}

void get_best_move(i32 *from_res, i32 *to_res, u64 *board, u32 *state_flags, i32 depth) {

}