cmake_minimum_required(VERSION 3.29)
project(chess_engine C)

set(CMAKE_C_STANDARD 11)

add_executable(chess_engine main.c
        move_gen.c
        types.h
        move_gen.h
        move_make.c
        move_make.h
        print.c
        print.h
        perft.c
        perft.h
        fen.c
        fen.h
        hash.c
        move_ordering.c
        move_ordering.h
        search.c
        search.h
        evaluation.c
        evaluation.h
        transposition.c
        transposition.h)

add_subdirectory(Google_tests)


