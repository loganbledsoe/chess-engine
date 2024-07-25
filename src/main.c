#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <time.h>
#include "board.h"
#include "circle.h"
#include "typedef.h"
#include "draw.h"

struct move {
    u32 from;
    u32 to;

    u32 time;

    struct move *next;
    struct move *prev;
};

typedef struct {
    u64 board[NUM_BITBOARDS];

    /* In order from least significant bit to most:
     * 1st: white queenside castle flag
     * 2nd: white kingside castle flag
     * 3rd: black queenside castle flag
     * 4th: black kingside castle flag
     * 
     * 5th-8th: index representing the
     * column that contains a pawn that
     * may be captured en passant. Cols
     * index from 0-7, 31 (max) for none.
     * 
     * 9th-15th: number representing
     * half move counter
     * 
     * 16th: 0 for white to move,
     * 1 for black to move
     * 
     * 17th-32nd: undefined, most likely
     * garbage values
    */
    u32 state_flags;

    u32 start_time;
    u32 increment_time;

    struct move *moves;
} game;

i32 main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initalize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *win = SDL_CreateWindow("Chess",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        720,
                                        720,
                                        SDL_WINDOW_SHOWN |
                                        SDL_WINDOW_RESIZABLE);
    if (!win) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    u32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, render_flags);
    if (!rend) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);

    u64 board[NUM_BITBOARDS] = {0x0000000000000010, // white king
                                0x1000000000000000, // black king
                                0x0000000000000008, // white queen
                                0x0800000000000000, // black queen
                                0x0000000000000081, // white rook
                                0x8100000000000000, // black rook
                                0x0000000000000024, // white bishop
                                0x2400000000000000, // black bishop
                                0x0000000000000042, // white knight
                                0x4200000000000000, // black knight
                                0x000000000000ff00, // white pawn
                                0x00ff000000000000, // black pawn
                                0x000000000000ffff, // white pieces
                                0xffff000000000000, // black pieces
                                0xffff00000000ffff, // all pieces
                                0x0000000000000000, // en passant
                                0xffffffffffffffff}; // unmoved

    u32 state_flags = 0xFF;
    init_masks();
    
    SDL_Texture *pieces[12];
    char path[30];
    // load pieces... TEMPORARY
    for (i32 i = 0; i < 12; i++) {
        sprintf(path, "assets/pieces/%d.svg", i);
        pieces[i] = IMG_LoadTexture(rend, path);

        if (!pieces[i]) {
            printf("Failed to create texture from surface: %s\n", SDL_GetError());
        }
    }

    SDL_Event event;

    i32 win_width;
    i32 win_height;

    i32 selected_pos = -1;
    u64 selected_pos_moves = 0;

    SDL_GetWindowSize(win, &win_width, &win_height);
    draw_frame(rend, win_width, win_height, board, pieces, selected_pos_moves);


    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) {
            break;
        }

        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                SDL_GetWindowSize(win, &win_width, &win_height);
                draw_frame(rend, win_width, win_height, board, pieces, selected_pos_moves);
            }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            i32 x_pos = 8 * event.button.x / win_height;
            i32 y_pos = 7 - 8 * event.button.y / win_height;
            if (x_pos < 8 && y_pos < 8) {
                i32 cur_pos = 8*y_pos + x_pos;
                if ((0x1ULL << cur_pos) & selected_pos_moves) {
                    move(board, &state_flags, selected_pos, cur_pos);
                    printf("----- DEBUG STATE FLAGS -----\n");
                    printf("White Castle Queenside: %s\n", (state_flags & 0x1) ? "TRUE" : "FALSE");
                    printf("White Castle Kingside: %s\n", (state_flags & 0x2) ? "TRUE" : "FALSE");
                    printf("Black Castle Queenside: %s\n", (state_flags & 0x4) ? "TRUE" : "FALSE");
                    printf("Black Castle Kingside: %s\n", (state_flags & 0x8) ? "TRUE" : "FALSE");
                    printf("Half Move Clock: %d\n", (state_flags >> 8) & 0x7F);
                    printf("%s to move\n", (state_flags & 0x8000) ? "BLACK" : "WHITE");
                    printf("EN PASSANT COLUMN: %d\n", (state_flags >> 4) & 0xF);
                    printf("-----------------------------\n");
                    selected_pos = -1;
                    selected_pos_moves = 0;

                    for (i32 i = 0; i < 10; ++i) {
                        evaluate(board, &state_flags);
                    }

                    draw_frame(rend, win_width, win_height, board, pieces, selected_pos_moves);

                    if (state_flags & 0x8000) {
                        u32 engine_start = SDL_GetTicks();
                        i32 best_from = 900;
                        i32 best_to = 900;
                        // get_best_move(&best_from, &best_to, board, &state_flags, 4);
                        printf("Best move: (%d, %d) found in %ds\n", best_from, best_to, (SDL_GetTicks() - engine_start)/1000);
                        move(board, &state_flags, best_from, best_to);
                    }
                } else {
                    selected_pos = cur_pos;
                    struct timespec start;
                    timespec_get(&start, TIME_UTC);
                    selected_pos_moves = getLegalMoves(board, &state_flags, selected_pos);
                    struct timespec end;
                    timespec_get(&end, TIME_UTC);
                    printf("Move generation took: %dns\n", end.tv_nsec - start.tv_nsec);
                }
                u32 start_time_2 = SDL_GetTicks();
                draw_frame(rend, win_width, win_height, board, pieces, selected_pos_moves);
                printf("Rendering took: %dms\n", SDL_GetTicks() - start_time_2);
            }
        }
    }

    // destroy piece textures
    for (i32 i = 0; i < 12; i++) {
        SDL_DestroyTexture(pieces[i]);
    }

    // destory other SDL stuff
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return EXIT_SUCCESS;
}