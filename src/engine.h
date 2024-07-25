#include "typedef.h"

extern void get_best_move(i32 *from_res, i32 *to_res, u64 *board, u32 *state_flags, i32 depth);
extern i32 evaluate(u64 *board, u32 *state_flags);