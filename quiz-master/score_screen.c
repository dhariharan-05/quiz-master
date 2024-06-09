#include "score_screen.h"

void showScorePage(WINDOW *win, int correct, int totalRows, int centredText, int windowWidth, int windowHeight) {
        werase(win);
        box(win,0,0);
        wmove(win,windowHeight - 5, 1);
        whline(win, ACS_HLINE, windowWidth - 2);
        wmove(win,windowHeight - 3, 1);
        mvwprintw(win, windowHeight - 3, centredText, "Quizmaster v1.0");
        mvwprintw(win, 5, 5, "SCORE: %d of %d", correct, totalRows);

        wrefresh(win);
        napms(2000);
}