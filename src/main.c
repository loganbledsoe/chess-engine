#include <pthread_time.h>
#include <stdio.h>
#include <string.h>

#include "evaluation.h"
#include "types.h"
#include "move.h"
#include "print.h"
#include "perft.h"
#include "fen.h"
#include "hash.h"
#include "search.h"
#include "transposition.h"

start_sq st_sq = {
    .w_q_rook = 0,
    .w_king = 4,
    .w_k_rook = 7,
    .b_q_rook = 56,
    .b_king = 60,
    .b_k_rook = 63,
};

void init() {
    init_hash();
    init_masks();
    init_evaluation();
}


// at the moment the main function is just used for testing
int main(void) {
    init();

    game_state state = {0};
    board bd = {0};

    char standard_start_pos[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    //char fen_str[] = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    //char fen_str[] = "2kr3r/Pppp1ppp/1b3nbN/nP6/BBPPP3/q4N2/P4RPP/q2Q2K1 w - d4 0 1";
    //char fen_str[] = "rnb1k2r/1pp1ppbp/6pn/1P1p2B1/p2PQ3/q1P5/4PPPP/RN2KBNR w KQkq - 1 10";
    //char fen_str[] = "r3kbnr/1pp2ppp/p1p1b3/4N2Q/3qP3/8/PPPP1PPP/RNB2RK1 b kq - 2 7";
    //char fen_str[] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    char fen_str[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    if (parse_fen(fen_str, &bd, &state)) {
        printf("Error parsing fen\n");
        return FAILURE;
    }

    board bd_ci = {0};
    game_state st_ci = {0};
    parse_fen(standard_start_pos, &bd_ci, &st_ci);
    init_castling_info(&bd_ci);

    print_board(&bd, &state);

    //perft_handler_details(&bd, &state, 5);
    perft_handler(&bd, &state, 6);

    // uint32_t depth = 8;
    // struct timespec start, end;
    // long seconds, nanoseconds;
    // double elapsed_ms;
    //
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // alpha_beta(&bd, &state, INT32_MIN, INT32_MAX, depth);
    // clock_gettime(CLOCK_MONOTONIC, &end);
    // seconds = end.tv_sec - start.tv_sec;
    // nanoseconds = end.tv_nsec - start.tv_nsec;
    // elapsed_ms = seconds * 1000.0 + nanoseconds / 1e6; // Convert to milliseconds
    // printf("Elapsed time: %.3f ms\n", elapsed_ms);
    // for (uint32_t i = 0; i < depth; ++i) {
    //     transposition* t = lookup_transposition(state.position_hash);
    //     print_move(t->move);
    //     printf("node type: %d", t->type);
    //     printf("\n");
    //     make_move(&bd, &state, t->move);
    // }

    return 0;
}
