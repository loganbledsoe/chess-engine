#include "board.h"
#include <stdio.h>

// remove -- for debuging purposes
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>

u64 ROW[8] = {ROW(0), ROW(1), ROW(2), ROW(3), ROW(4), ROW(5), ROW(6), ROW(7)}; // rows masks
u64 COL[8] = {COL(0), COL(1), COL(2), COL(3), COL(4), COL(5), COL(6), COL(7)}; // column masks
u64 DAG[15]; // diagonal masks
u64 ADG[15]; // anti-diagonal masks

i32 abs(i32 x) {
    if (x >= 0)
        return x;
    return -x;
}

void init_masks() {
    DAG[7] = 0x8040201008040201ULL;
    ADG[7] = 0x0102040810204080ULL;
    for (i32 i = 6; i >= 0; --i) {
        DAG[i] = (DAG[i + 1] >> 1) & ~COL(7);
        ADG[i] = (ADG[i + 1] >> 1) & ~COL(7);
    }
    for (i32 i = 8; i < 15; ++i) {
        DAG[i] = (DAG[i - 1] << 1) & ~COL(0);
        ADG[i] = (ADG[i - 1] << 1) & ~COL(0);
    }
}

u64 reverse(u64 a) {
    a = ((a & 0xffffffff00000000ULL) >> 32) | ((a & 0x00000000ffffffffULL) << 32); // can remove mask on this line
    a = ((a & 0xffff0000ffff0000ULL) >> 16) | ((a & 0x0000ffff0000ffffULL) << 16);
    a = ((a & 0xff00ff00ff00ff00ULL) >> 8) | ((a & 0x00ff00ff00ff00ffULL) << 8);
    a = ((a & 0xf0f0f0f0f0f0f0f0ULL) >> 4) | ((a & 0x0f0f0f0f0f0f0f0fULL) << 4);
    a = ((a & 0xccccccccccccccccULL) >> 2) | ((a & 0x3333333333333333ULL) << 2);
    a = ((a & 0xaaaaaaaaaaaaaaaaULL) >> 1) | ((a & 0x5555555555555555ULL) << 1);
    return a;
}

u64 xorshift64star(void) {
    static u64 x = 0x2C1929944248EEULL; // seed
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

u64 row_col(u64 occ, i32 pos) {
    u64 rk = (0x1ULL << pos);

    u64 row = occ & ROW[pos / 8];
    row = (row - 2 * rk) ^ reverse(reverse(row) - 2 * reverse(rk));
    row &= ROW[pos / 8];
    
    u64 col = occ & COL[pos % 8];
    col = (col - 2 * rk) ^ reverse(reverse(col) - 2 * reverse(rk));
    col &= COL[pos % 8];

    return row | col;
}

// hyperbola quintessence
u64 diag_adiag(u64 occ, i32 pos) {
    u64 rk = (0x1ULL << pos);

    u64 row = occ & DAG[7 - (pos / 8) + (pos % 8)];
    row = (row - 2 * rk) ^ reverse(reverse(row) - 2 * reverse(rk));
    row &= DAG[7 - (pos / 8) + (pos % 8)];
    
    u64 col = occ & ADG[(pos / 8) + (pos % 8)];
    col = (col - 2 * rk) ^ reverse(reverse(col) - 2 * reverse(rk));
    col &= ADG[(pos / 8) + (pos % 8)];

    return row | col;
}

    /* Returns a momento, a u32 where the 15
     * least significant bits match
     * state_flags
     * 
     * 17th-22st: from position
     * 23nd-28th: to position
     * 29th-32st: captured type, 15 for none
    */
u32 move(u64 *board, u32 *state_flags, u32 from, u32 to) {
    if (from > 63 || from < 0 || to > 63 || to < 0)
        printf("MOVE BOUND ERROR!!!");
    i32 from_type = NONE;
    i32 to_type = NONE;
    for (i32 i = 0; i < 12; ++i) {
        if (board[i] & (0x1ULL << from))
            from_type = i;
        if (board[i] & (0x1ULL << to))
            to_type = i;
    }

    // create momento
    //printf("TO TYPE: %x\n", to_type << 28);
    u32 momento = (to_type << 28) | (to << 22) | (from << 16);
    momento |= *state_flags & 0xffff;

    // set castle flags
    if (from == 0 || to == 0) {
        *state_flags &= ~0x1;
    } else if (from == 7 || to ==7) {
        *state_flags &= ~0x2;
    } else if (from == 56 || to == 56) {
        *state_flags &= ~0x4;
    } else if (from == 63 || to == 63) {
        *state_flags &= ~0x8;
    } else if (from == 4) {
        *state_flags &= ~0x3;
    } else if (from == 60) {
        *state_flags &= ~0xC;
    }

    // set half move clock
    if ((board[ALL] & (0x1ULL << to)) ||
        from_type == W_PAWN || from_type == B_PAWN) {
            *state_flags &= ~0x7F00;
    } else {
        *state_flags += 0x100;
    }

    // set team to move
    *state_flags ^= 0x8000;

    u64 enpas = 0;
    if ((*state_flags & 0xF0) != 0xF0) {
        enpas = COL[(*state_flags >> 4) & 0xF] & (ROW[3] | ROW[4]);
    }

    // en passant capture as white
    if ((from_type == W_PAWN) && ((to - from) % 2) &&
        (enpas & (0x1ULL << (to - 8))) && to_type == NONE) {
            enpas &= ROW[4];
            board[B_PAWN] = board[B_PAWN] ^ enpas;
            board[B_ALL] = board[B_ALL] ^ enpas;
    // en passant capture as black
    } else if (from_type == B_PAWN && ((from - to) % 2) &&
        (enpas & (0x1ULL << (to + 8))) && to_type == NONE) {
            enpas &= ROW[3];
            board[W_PAWN] = board[W_PAWN] ^ enpas;
            board[W_ALL] = board[W_ALL] ^ enpas;
    }

    // update en passant flags
    if (from_type == W_PAWN && (to - from) == 16) {
        *state_flags &= ~0xF0;
        *state_flags |= (to % 8) << 4;
    } else if (from_type == B_PAWN && (from - to) == 16) {
        *state_flags &= ~0xF0;
        *state_flags |= (to % 8) << 4;
    } else {
        *state_flags |= 0xF0;
    }

    // update moved piece type board
    board[from_type] = (board[from_type] | (0x1ULL << to)) ^ (0x1ULL << from);
    board[(from_type % 2) + W_ALL] = (board[(from_type % 2) + W_ALL] | (0x1ULL << to)) ^ (0x1ULL << from);

    // update captured piece type board
    if (to_type != NONE) {
        board[to_type] = board[to_type] ^ (0x1ULL << to);
        board[((from_type + 1)% 2) + W_ALL] = board[((from_type + 1)% 2) + W_ALL] ^ (0x1ULL << to);
    }

    // update all board
    board[ALL] = board[W_ALL] | board[B_ALL];

    return momento;
}

void undo_move (u64 *board, u32 *state_flags, u32 momento) {
    *state_flags = momento;
    i32 from = (momento >> 16) & 0x3F;
    i32 to = (momento >> 22) & 0x3F;
    i32 captured_type =  (momento >> 28) & 0xF;

    if (from > 63 || from < 0 || to > 63 || to < 0)
        printf("UNDO MOVE BOUND ERROR!!!");

    i32 to_type;
    for (i32 i = 0; i < 12; ++i) {
        if (board[i] & (0x1ULL << to))
            to_type = i;
    }

    // move piece back
    board[to_type] = (board[to_type] | (0x1ULL << from)) ^ (0x1ULL << to);
    board[(to_type % 2) + W_ALL] = (board[(to_type % 2) + W_ALL] | (0x1ULL << from)) ^ (0x1ULL << to);

    // replace captured piece
    if (captured_type != 0xF) {
        board[captured_type] = board[captured_type] | (0x1ULL << to);
        board[(captured_type % 2) + W_ALL] = board[(captured_type % 2) + W_ALL] | (0x1ULL << to);
    }

    // replace captured piece en passant
    if (captured_type == 0xF) {
        // capture as white
        if ((to_type == W_PAWN) && ((to - from) % 2)) {
            board[B_PAWN] |= ((0x1ULL << to) >> 8);
            board[B_ALL] |= board[B_PAWN];
        // capture as black
        } else if ((to_type == B_PAWN) && ((from - to) % 2)) {
            board[W_PAWN] |= ((0x1ULL << to + 8));
            board[W_ALL] |= board[W_PAWN];
        }
    }

    // update all board
    board[ALL] = board[W_ALL] | board[B_ALL];
}

u64 knight(i32 pos) {
    u64 kn = 0x1ULL << pos;
    u64 moves = 0;
    moves |= (kn << 6) & ~(COL(6) | COL(7));
    moves |= (kn >> 6) & ~(COL(0) | COL(1));
    moves |= (kn << 10) & ~(COL(0) | COL(1));
    moves |= (kn >> 10) & ~(COL(6) | COL(7));
    moves |= (kn << 15) & ~COL(7);
    moves |= (kn >> 15) & ~COL(0);
    moves |= (kn << 17) & ~COL(0);
    moves |= (kn >> 17) & ~COL(7);
    return moves;
}

u64 pawn_w(u64 occ, u32 state_flags, i32 pos) {
    u64 pawn = 0x1ULL << pos;
    // forward one
    u64 moves = pawn << 8;
    moves &= ~occ;

    // forward two
    if (moves && (pawn & ROW[1])) {
        moves |= moves << 8;
        moves &= ~occ;
    }

    u64 enpas = 0;
    // if ((state_flags & 0xF0) != 0xF0) {
    //     enpas = COL[(state_flags >> 4) & 0xF] & ROW[4];
    // }

    // capture (includes en passant)
    moves |= (pawn << 7) & (occ | (enpas << 8)) & ~COL(7);
    moves |= (pawn << 9) & (occ | (enpas << 8)) & ~COL(0);
    return moves;
}

u64 pawn_b(u64 occ, u32 state_flags, i32 pos) {
    u64 pawn = 0x1ULL << pos;
    // forward one
    u64 moves = pawn >> 8;
    moves &= ~occ;

    // forward two
    if (moves && (pawn & ROW[6])) {
        moves |= moves >> 8;
        moves &= ~occ;
    }

    u64 enpas = 0;
    // if ((state_flags & 0xF0) != 0xF0) {
    //     enpas = COL[(state_flags >> 4) & 0xF] & ROW[3];
    // }

    // capture (includes en passant)
    moves |= (pawn >> 7) & (occ | (enpas >> 8)) & ~COL(0);
    moves |= (pawn >> 9) & (occ | (enpas >> 8)) & ~COL(7);
    return moves;
}

u64 king(i32 pos) {
    u64 kn = 0x1ULL << pos;
    u64 moves_left = ((kn >> 1) | (kn >> 9) | (kn << 7));
    moves_left &= ~COL(7);
    u64 moves_right = ((kn << 1) | (kn << 9) | (kn >> 7));
    moves_right &= ~COL(0);
    return moves_left | moves_right | (kn << 8) | (kn >> 8);
}

u64 get_threats(u64 *board, i32 pos) {
    i32 team = WHITE_TEAM;
    if ((board[W_ALL] & (0x1ULL << pos)) != 0) {
        team = BLACK_TEAM;
    }

    u64 threats = 0;
    threats |= board[W_KING + team] & king(pos);
    threats |= (board[W_QUEEN + team] | board[W_ROOK + team]) & row_col(board[ALL], pos);
    threats |= (board[W_QUEEN + team] | board[W_BISHOP + team]) & diag_adiag(board[ALL], pos);

    if (team) {
        threats |= ((0x1ULL << pos) << 7) & ~COL(7) & board[B_PAWN];
        threats |= ((0x1ULL << pos) << 9) & ~COL(0) & board[B_PAWN];
    } else {
        threats |= ((0x1ULL << pos) >> 7) & ~COL(0) & board[W_PAWN];
        threats |= ((0x1ULL << pos) >> 9) & ~COL(7) & board[W_PAWN];
    }
    return threats;
}

u64 getLegalMoves(u64 *board, u32 *state_flags, i32 pos) {
    i32 type = NONE;
    for (i32 i = 0; i < 12; ++i) {
        if (board[i] & (0x1ULL << pos))
            type = i;
    }
    u64 pseudo_legal_moves = getPseudoLegalMoves(board, *state_flags, pos, type);
    u64 legal_moves = 0;
    i32 to;
    i32 momento;
    i32 king_pos;
    while (pseudo_legal_moves) {
        to = __builtin_ctzll(pseudo_legal_moves);
        pseudo_legal_moves ^= 0x1ULL << to;
        momento = move(board, state_flags, pos, to);
        king_pos = __builtin_ctzll(board[type % 2]);
        if (!get_threats(board, king_pos))
            legal_moves |= 0x1ULL << to;
        undo_move(board, state_flags, momento);
    }
    return legal_moves;
}


u64 getPseudoLegalMoves(u64 *board, u32 state_flags, i32 pos, i32 type) {
    switch (type) {
        case NONE:
            return 0;
        case W_KING:
            return king(pos) & ~board[W_ALL];
        case B_KING:
            return king(pos) & ~board[B_ALL];
        case W_QUEEN:
            return (row_col(board[ALL], pos) | diag_adiag(board[ALL], pos)) & ~board[W_ALL];
        case B_QUEEN:
            return (row_col(board[ALL], pos) | diag_adiag(board[ALL], pos)) & ~board[B_ALL];
        case W_ROOK:
            return row_col(board[ALL], pos) & ~board[W_ALL];
        case B_ROOK:
            return row_col(board[ALL], pos) & ~board[B_ALL];
        case W_BISHOP:
            return diag_adiag(board[ALL], pos) & ~board[W_ALL];
        case B_BISHOP:
            return diag_adiag(board[ALL], pos) & ~board[B_ALL];
        case W_KNIGHT:
            return knight(pos) & ~board[W_ALL];
        case B_KNIGHT:
            return knight(pos) & ~board[B_ALL];
        case W_PAWN:
            return pawn_w(board[ALL], state_flags, pos) & ~board[W_ALL];
        case B_PAWN:
            return pawn_b(board[ALL], state_flags, pos) & ~board[B_ALL];
    }
}