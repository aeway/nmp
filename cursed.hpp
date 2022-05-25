#pragma once

#include <ncurses.h>

// CP = COLOR PAIR
constexpr int DIRECTORY_CP = 1;
constexpr int PLAYING_CP = 2;
constexpr int SELECTED_WINDOW_CP = 3;


#define START_NCURSES                                       \
    initscr();                                              \
    noecho();                                               \
    cbreak();                                               \
    curs_set(0);                                            \
    keypad(stdscr, 1);                                      \
    start_color();                                          \
    init_pair(DIRECTORY_CP, COLOR_BLUE, COLOR_BLACK);       \
    init_pair(PLAYING_CP, COLOR_GREEN, COLOR_BLACK);        \
    init_pair(SELECTED_WINDOW_CP, COLOR_BLUE, COLOR_BLACK);
