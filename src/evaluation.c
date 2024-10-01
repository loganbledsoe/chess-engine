//
// Created by Logan on 9/26/2024.
//

#include "evaluation.h"

#include <stdio.h>
#include <string.h>

#include "print.h"

int8_t piece_sq[TYPE_NB + 1][SQ_NB] = {0};

void init_evaluation(void) {
    int8_t w_pawn[SQ_NB] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };
    memcpy(piece_sq[W_PAWN], w_pawn, SQ_NB * sizeof(int8_t));

    int8_t w_knight[SQ_NB] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    };
    memcpy(piece_sq[W_KNIGHT], w_knight, SQ_NB * sizeof(int8_t));

    int8_t w_bishop[SQ_NB] = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    };
    memcpy(piece_sq[W_BISHOP], w_bishop, SQ_NB * sizeof(int8_t));

    int8_t w_rook[SQ_NB] = {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    };
    memcpy(piece_sq[W_ROOK], w_rook, SQ_NB * sizeof(int8_t));

    int8_t w_queen[SQ_NB] = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
         0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };
    memcpy(piece_sq[W_QUEEN], w_queen, SQ_NB * sizeof(int8_t));

    int8_t w_king[SQ_NB] = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    };
    memcpy(piece_sq[W_KING], w_king, SQ_NB * sizeof(int8_t));

    for (int type = B_KING; type < TYPE_NB; type += 2) {
        for (int sq = 0; sq < SQ_NB; ++sq) {
            piece_sq[type][sq] = -piece_sq[type - 1][SQ_NB - 1 - sq];
        }
    }
}

int32_t evaluate_board(board *bd, game_state *st) {
    static int32_t piece_values[TYPE_NB + 1] = {
        20000, -20000, 900, -900, 500, -500, 330, -330, 320, -320, 100, -100, 0
    };

    int32_t eval = 0;
    for (uint8_t sq = 0; sq < SQ_NB; ++sq) {
        eval += piece_values[bd->type_arr[sq]];
        eval += piece_sq[bd->type_arr[sq]][sq];
    }
    eval = eval * (st->team_to_move == WHITE) - eval * (st->team_to_move == BLACK);
    return eval;
}