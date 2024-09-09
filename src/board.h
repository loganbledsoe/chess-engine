#include "typedef.h"

#define NONE -1
#define W_KING 0 // white king
#define B_KING 1 // black king
#define W_QUEEN 2 // white queen
#define B_QUEEN 3 // black queen
#define W_ROOK 4 // white rook
#define B_ROOK 5 // black rook
#define W_BISHOP 6 // white bishop
#define B_BISHOP 7 // black bishop
#define W_KNIGHT 8 // white knight
#define B_KNIGHT 9 // black knight
#define W_PAWN 10 // white pawn
#define B_PAWN 11 // black pawn

#define W_ALL 12 // white piece
#define B_ALL 13 // black piece
#define ALL 14 // occupancy

#define NUM_BITBOARDS 15
#define NUM_PIECE_TYPES 12

#define WHITE_TEAM 0
#define BLACK_TEAM 1

#define COL(i) (0x0101010101010101ULL << i)
#define ROW(i) (0x00000000000000ffULL << (8 * i))

extern void init_masks(void);