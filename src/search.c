//
// Created by Logan on 9/26/2024.
//

#include "search.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evaluation.h"
#include "move_gen.h"
#include "move_make.h"
#include "move_ordering.h"
#include "print.h"
#include "transposition.h"

static uint8_t is_capture(uint16_t move) {
    return (move >> 14) & 0x1;
}

static void sort(uint16_t *move_list, uint16_t *move_scores, uint32_t num_moves, uint32_t idx) {
    int16_t best_move_idx = idx;
    for (uint32_t j = idx + 1; j < num_moves; ++j) {
        if (move_scores[j] > move_scores[best_move_idx]) {
            best_move_idx = j;
        }
    }
    // swap moves
    uint16_t temp_move = move_list[idx];
    move_list[idx] = move_list[best_move_idx];
    move_list[best_move_idx] = temp_move;
    uint16_t temp_score = move_scores[idx];
    move_scores[idx] = move_scores[best_move_idx];
    move_scores[best_move_idx] = temp_score;
}

int32_t quiesce(board *bd, game_state *st, int32_t alpha, int32_t beta) {
    int stand_pat = evaluate_board(bd, st);
    if (stand_pat >= beta)
        return beta;
    if (alpha < stand_pat)
        alpha = stand_pat;

    uint16_t move_list[256];
    uint32_t num_moves = get_move_list(bd, st, move_list);
    uint32_t num_skipped = 0;
    for (uint32_t i = 0; i < num_moves; ++i) {
        if (!is_capture(move_list[i])) {
            continue;
        }
        game_state momento;
        memcpy(&momento, st, sizeof(game_state));
        make_move(bd, st, move_list[i]);
        if (get_threats(bd, st, __builtin_ctzll(bd->type_bb[!st->team_to_move]), !st->team_to_move)) {
            unmake_move(bd, st, &momento);
            num_skipped++;
            continue;
        }

        int32_t score = -quiesce(bd, st, -beta, -alpha);
        unmake_move(bd, st, &momento);

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }
    if (num_skipped == num_moves) {
        if (st->in_check) {
            // checkmate
            return INT32_MIN;
        }
        // stalemate
        return 0;
    }
    return alpha;
}


int32_t alpha_beta(board *bd, game_state *st, int32_t alpha, int32_t beta, int32_t depth) {
    transposition *transpos = lookup_transposition(st->position_hash);
    if (transpos && transpos->depth >= depth) {
        switch (transpos->type) {
            case EXACT:
                return transpos->score;
            case LOWER_BOUND:
                if (transpos->score > alpha) {
                    alpha = transpos->score;
                }
                break;
            case UPPER_BOUND:
                if (transpos->score < beta) {
                    beta = transpos->score;
                }
                break;
        }
        if (alpha >= beta) {
            return transpos->score;
        }
    }

    if (depth == 0) {
        return quiesce(bd, st, alpha, beta);
    }
    int32_t best_score = INT32_MIN;
    uint16_t move_list[256];
    uint32_t num_moves = get_move_list(bd, st, move_list);
    uint32_t num_skipped = 0;
    uint16_t move_scores[256];
    score_moves(bd, st, move_list, move_scores, num_moves);
    uint16_t best_move;
    uint8_t node_type = UPPER_BOUND;
    for (uint32_t i = 0; i < num_moves; ++i) {
        // one step in selection sort
        sort(move_list, move_scores, num_moves, i);
        game_state momento;
        memcpy(&momento, st, sizeof(game_state));
        make_move(bd, st, move_list[i]);
        // verify move is legal -- does not put own king in check
        if (get_threats(bd, st, __builtin_ctzll(bd->type_bb[!st->team_to_move]), !st->team_to_move)) {
            unmake_move(bd, st, &momento);
            num_skipped++;
            continue;
        }
        int32_t score = -alpha_beta(bd, st, -beta, -alpha, depth - 1);
        unmake_move(bd, st, &momento);
        // if (depth == 8) {
        //     print_move(move_list[i]);
        //     printf("score: %d\n\n", score);
        // }
        if (score > best_score) {
            best_score = score;
            best_move = move_list[i];
            if (score > alpha) {
                alpha = score;
                node_type = EXACT;
            }
        }
        if (score >= beta) {
            node_type = LOWER_BOUND;
            break;
        }
    }
    if (num_skipped == num_moves) {
        if (st->in_check) {
            return INT32_MIN;
        }
        return 0;
    }

    record_transposition(st->position_hash, best_move, best_score, node_type, depth);
    return best_score;
}

// not needed if pulling pv from transposition table
uint16_t search(board *bd, game_state *st, uint32_t depth) {
    //print_board(bd, st);
    int32_t alpha = INT32_MIN;
    int32_t beta = INT32_MAX;

    int32_t best_score = INT32_MIN;
    uint16_t best_move;

    uint16_t move_list[256];
    uint32_t num_moves = get_move_list(bd, st, move_list);
    for (uint32_t i = 0; i < num_moves; ++i) {
        game_state momento;
        memcpy(&momento, st, sizeof(game_state));
        make_move(bd, st, move_list[i]);
        if (get_threats(bd, st, __builtin_ctzll(bd->type_bb[!st->team_to_move]), !st->team_to_move)) {
            unmake_move(bd, st, &momento);
            continue;
        }
        int32_t score = -alpha_beta(bd, st, -beta, -alpha, depth - 1);
        unmake_move(bd, st, &momento);

        if (score > best_score) {
            best_score = score;
            best_move = move_list[i];
            if (score > alpha) {
                alpha = score;
            }
        }
        if (score >= beta) {
            return best_move;
        }
    }
    //printf("Eval: %d\n", best_score);
    return best_move;
}
