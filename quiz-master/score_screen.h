//score-screen.h
#ifndef SCORE_SCREEN_H
#define SCORE_SCREEN_H

#include <ncurses.h>
#include <stdlib.h>

void showScorePage(WINDOW *win, int correct, int totalRows, int centredText, int windowWidth, int winddowHeight);

#endif