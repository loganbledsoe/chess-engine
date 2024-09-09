#include "board.h"

u64 ROW[8] = {ROW(0), ROW(1), ROW(2), ROW(3), ROW(4), ROW(5), ROW(6), ROW(7)}; // rows masks
u64 COL[8] = {COL(0), COL(1), COL(2), COL(3), COL(4), COL(5), COL(6), COL(7)}; // column masks
u64 DAG[15]; // diagonal masks
u64 ADG[15]; // anti-diagonal masks

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

// --- piece type moves --- //

u64 get_moves_knight(u64 pos) {
    u64 moves = 0x0ULL;
    moves |= (pos << 6) & ~(COL(6) | COL(7));
    moves |= (pos >> 6) & ~(COL(0) | COL(1));
    moves |= (pos << 10) & ~(COL(0) | COL(1));
    moves |= (pos >> 10) & ~(COL(6) | COL(7));
    moves |= (pos << 15) & ~COL(7);
    moves |= (pos >> 15) & ~COL(0);
    moves |= (pos << 17) & ~COL(0);
    moves |= (pos >> 17) & ~COL(7);
    return moves;
}

u64 get_moves_pawn_w(u64 pos, u64 occ, u32 state_flags) {
    // forward one
    u64 moves = pos << 8;
    moves &= ~occ;

    // forward two
    if (moves && (pos & ROW[1])) {
        moves |= moves << 8;
        moves &= ~occ;
    }

    u64 enpas = 0x0ULL;
    if ((state_flags & 0xF0) != 0xF0) {
        enpas = COL[(state_flags >> 4) & 0xF] & ROW[4];
    }

    // capture (includes en passant)
    moves |= (pos << 7) & (occ | (enpas << 8)) & ~COL(7);
    moves |= (pos << 9) & (occ | (enpas << 8)) & ~COL(0);
    return moves;
}

u64 get_moves_pawn_b(u64 pos, u64 occ, u32 state_flags) {
    // forward one
    u64 moves = pos >> 8;
    moves &= ~occ;

    // forward two
    if (moves && (pos & ROW[6])) {
        moves |= moves >> 8;
        moves &= ~occ;
    }

    u64 enpas = 0x0ULL;
    if ((state_flags & 0xF0) != 0xF0) {
        enpas = COL[(state_flags >> 4) & 0xF] & ROW[3];
    }

    // capture (includes en passant)
    moves |= (pos >> 7) & (occ | (enpas >> 8)) & ~COL(0);
    moves |= (pos >> 9) & (occ | (enpas >> 8)) & ~COL(7);
    return moves;
}

u64 get_moves_king(u64 pos) {
    u64 moves_left = ((pos >> 1) | (pos >> 9) | (pos << 7));
    moves_left &= ~COL(7);
    u64 moves_right = ((pos << 1) | (pos << 9) | (pos >> 7));
    moves_right &= ~COL(0);
    return moves_left | moves_right | (pos << 8) | (pos >> 8);
}

u64 get_moves_rook(u64 pos, u64 occ) {
    u64 row = occ & ROW[pos / 8];
    row = (row - 2 * pos) ^ reverse(reverse(row) - 2 * reverse(pos));
    row &= ROW[pos / 8];
    
    u64 col = occ & COL[pos % 8];
    col = (col - 2 * pos) ^ reverse(reverse(col) - 2 * reverse(pos));
    col &= COL[pos % 8];

    return row | col;
}

u64 get_moves_bishop(u64 pos, u64 occ) {
    u64 row = occ & DAG[7 - (pos / 8) + (pos % 8)];
    row = (row - 2 * pos) ^ reverse(reverse(row) - 2 * reverse(pos));
    row &= DAG[7 - (pos / 8) + (pos % 8)];
    
    u64 col = occ & ADG[(pos / 8) + (pos % 8)];
    col = (col - 2 * pos) ^ reverse(reverse(col) - 2 * reverse(pos));
    col &= ADG[(pos / 8) + (pos % 8)];

    return row | col;
}

// returns the pseudo legal moves for a given position as a bitboard
u64 get_moves(u64 *board, u32 state_flags, i32 pos, i32 type) {
    if (type == NONE) {
        return 0x0ULL;
    }

    u64 moves;
    switch (type) {
        case W_KING:
        case B_KING:
            moves = get_moves_king(pos);
            break;
        case W_QUEEN:
        case B_QUEEN:
            moves = get_moves_rook(pos, board[ALL]);
            moves |= get_moves_bishop(pos, board[ALL]);
            break;
        case W_ROOK:
        case B_ROOK:
            moves = get_moves_rook(pos, board[ALL]);
            break;
        case W_BISHOP:
        case B_BISHOP:
            moves = get_moves_bishop(pos, board[ALL]);
            break;
        case W_KNIGHT:
        case B_KNIGHT:
            moves = get_moves_knight(pos);
            break;
        case W_PAWN:
            return get_moves_pawn_w(pos, board[ALL], state_flags);
        case B_PAWN:
            return get_moves_pawn_b(pos, board[ALL], state_flags);
    }

    if (type % 2) {
        moves &= ~board[B_ALL];
    } else {
        moves &= ~board[W_ALL];
    }
    return moves;
}


// move:

// returns the number of moves
u32 get_moves_list(u64 *board, u32 state_flags, i32 pos, i32 type, u16 *list) {
    u64 moves = get_moves(board, state_flags, pos, type);
    i32 to;

    i32 idx = 0;
    while (moves) {
    to = __builtin_ctzll(moves);
    moves ^= 0x1ULL << to;
    list[idx] = pos;
    list[idx] |= to << 6;
    ++idx;
    }
    return idx;
}

u32 get_all_moves_list(u64 *board, u32 state_flags, i32 pos, i32 type) {
    u32 num_moves;
    u16 moves[64];
    // do this for all 
    num_moves = get_moves_list(board, state_flags, pos, type, &moves);

}