#include "draw.h"

void draw_frame(SDL_Renderer *rend, i32 w, i32 h, u64 *board, SDL_Texture **pieces, u64 highlighted_moves) {
    // draw board
    SDL_SetRenderDrawColor(rend, 34, 40, 49, 255);
        SDL_RenderClear(rend);

        i32 y_off = h - 8 * (h / 8);
        SDL_Rect sq = {0, 0, h/8, h/8};
        u32 color = 0;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (color)
                    SDL_SetRenderDrawColor(rend, 119, 140, 82, 255);
                else
                    SDL_SetRenderDrawColor(rend, 235, 235, 206, 255);
                color = !color;

                SDL_RenderFillRect(rend, &sq);
                sq.x += h/8;
            }
            // in case screen height is not divisible by 8
            if (8 - i - 1 == y_off)
                sq.h += 1;
            if (8 - i <= y_off)
                sq.y += 1;
            color = !color;
            sq.y += h/8;
            sq.x = 0;
        }

        // draw pieces
        sq.h = h/8;
        for (i32 j = 0; j < 64; ++j) {
            sq.x = sq.h * (j % 8);
            sq.y = sq.h * (7 - j / 8);
            // if ((highlighted_moves >> j) & 0x1ULL) {
            //     SDL_SetRenderDrawColor(rend, 118, 171, 174, 160);
            //     SDL_RenderFillRect(rend, &sq);
            // }
            for (i32 i = 0; i < 12; ++i) {
                if ((board[i] >> j) & 0x1ULL) {
                    SDL_RenderCopy(rend, pieces[i], NULL, &sq);
                }
            }
        }

        //draw highlights
        SDL_SetRenderDrawColor(rend, 118, 171, 174, 160);   
        for (i32 i = 0; i < 64; ++i) {
            if ((highlighted_moves >> i) & 0x1ULL) {
                drawCircle(rend, (i%8)*(h/8) + h/16, (8 - i/8)*(h/8) - h/16, h/64);
            }
        }

        // ---------- debug -------------  
        for (i32 i = 0; i < 64; ++i) {
            if ((board[14] >> i) & 0x1ULL) {
                SDL_SetRenderDrawColor(rend, 255, 0, 0, 160);
                drawCircle(rend, (i%8)*(h/8) + h/32, (8 - i/8)*(h/8) - 3*h/32, h/64);
            }
            if ((board[12] >> i) & 0x1ULL) {
                SDL_SetRenderDrawColor(rend, 255, 255, 255, 160); 
                drawCircle(rend, (i%8)*(h/8) + h/32, (8 - i/8)*(h/8) - h/32, h/64);
            }
            if ((board[13] >> i) & 0x1ULL) {
                SDL_SetRenderDrawColor(rend, 0, 0, 0, 160);
                drawCircle(rend, (i%8)*(h/8) + 3*h/32, (8 - i/8)*(h/8) - h/32, h/64);
            }
        }
        
        SDL_RenderPresent(rend);
}