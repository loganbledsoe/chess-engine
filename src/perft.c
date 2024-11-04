//
// Created by Logan on 9/13/2024.
//

#include "perft.h"

#include <pthread_time.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "move.h"
#include "print.h"

// checks if flags match double pawn push
static uint8_t is_double_pawn_push(uint8_t flags) {
    return flags == 0x1;
}

// checks if flags match en passant capture
static uint8_t is_en_passant(uint8_t flags) {
    return flags == 0x5;
}

// checks if flags match castle
static uint8_t is_castle(uint8_t flags) {
    return flags == 0x2 || flags == 0x3;
}

// checks if flags match capture
static uint8_t is_capture(uint8_t flags) {
    return (flags & 0x4) >> 2;
}

// checks if flags match promotion
static uint8_t is_promotion(uint8_t flags) {
    return (flags & 0x8) >> 3;
}

// The check for checkmate in the leaf node makes this particular perft implementation slow.
// It should be used for testing accuracy, not performance.
void perft_detailed(perft_results *results, board *bd, game_state *state, uint32_t depth) {
    if (depth == 0) {
        results->nodes += 1;
        results->captures += is_capture(get_flags(state->last_move));
        results->en_passant += is_en_passant(get_flags(state->last_move));
        results->castles += is_castle(get_flags(state->last_move));
        results->promotions += is_promotion(get_flags(state->last_move));
        results->checks += state->in_check != 0;

        move_list mv_list;
        get_move_list(bd, state, &mv_list);
        remove_illegal_moves(bd, state, &mv_list);
        results->checkmates += (state->in_check != 0) && (mv_list.size == 0);

        return;
    }
    move_list mv_list;
    get_move_list(bd, state, &mv_list);
    game_state momento;
    for (uint32_t i = 0; i < mv_list.size; i++) {
        memcpy(&momento, state, sizeof(game_state));
        make_move(bd, state, mv_list.moves[i]);
        if (!get_threats(bd, state, __builtin_ctzll(bd->type_bb[!state->team_to_move]), !state->team_to_move)) {
            perft_detailed(results, bd, state, depth - 1);
        }
        unmake_move(bd, state, &momento);
    }
}

// this perft implementation only returns node count and should be used for performance testing
uint64_t perft(board *bd, game_state *state, uint32_t depth) {
    if (depth == 0) {
        return 0x1ULL;
    }
    uint64_t num_nodes = 0ULL;
    move_list mv_list;
    get_move_list(bd, state, &mv_list);
    game_state momento;
    for (uint32_t i = 0; i < mv_list.size; i++) {
        memcpy(&momento, state, sizeof(game_state));
        make_move(bd, state, mv_list.moves[i]);
        if (!get_threats(bd, state, __builtin_ctzll(bd->type_bb[!state->team_to_move]), !state->team_to_move)) {
            num_nodes += perft(bd, state, depth - 1);
        }
        unmake_move(bd, state, &momento);
    }
    return num_nodes;
}

void perft_handler_details(board *bd, game_state *state, uint32_t depth) {
    struct timespec start, end;
    long seconds, nanoseconds;
    double elapsed_ms;

    perft_results results = {0};

    clock_gettime(CLOCK_MONOTONIC, &start);
    perft_detailed(&results, bd, state, depth);
    clock_gettime(CLOCK_MONOTONIC, &end);

    seconds = end.tv_sec - start.tv_sec;
    nanoseconds = end.tv_nsec - start.tv_nsec;
    elapsed_ms = seconds * 1000.0 + nanoseconds / 1e6; // Convert to milliseconds

    printf("--------------------------\n");
    printf("PERFT RESULTS:\n\n");
    printf("Elapsed time: %.3f ms\n", elapsed_ms);
    printf("Depth:        %d\n\n", depth);
    printf("Nodes:        %llu\n", results.nodes);
    printf("Captures:     %llu\n", results.captures);
    printf("En Passant:   %llu\n", results.en_passant);
    printf("Castles:      %llu\n", results.castles);
    printf("Promotions:   %llu\n", results.promotions);
    printf("Checks:       %llu\n", results.checks);
    printf("Checkmates:   %llu\n", results.checkmates);
    printf("--------------------------\n");
}

void perft_handler(board *bd, game_state *state, uint32_t depth) {
    struct timespec start, end;
    long seconds, nanoseconds;
    double elapsed_ms;

    clock_gettime(CLOCK_MONOTONIC, &start);
    uint64_t node_count = perft(bd, state, depth);
    clock_gettime(CLOCK_MONOTONIC, &end);

    seconds = end.tv_sec - start.tv_sec;
    nanoseconds = end.tv_nsec - start.tv_nsec;
    elapsed_ms = seconds * 1000.0 + nanoseconds / 1e6; // Convert to milliseconds

    printf("--------------------------\n");
    printf("PERFT RESULTS:\n\n");
    printf("Elapsed time: %.3f ms\n", elapsed_ms);
    printf("Depth:        %d\n\n", depth);
    printf("Nodes:        %llu\n", node_count);
    printf("--------------------------\n");
}