#include "welcome_screen.h"

#define START_OPTION 0
#define LEAVE_OPTION 1

void showWelcomePage(WINDOW *win) {
    
    mvwprintw(win, 5, 5, "Welcome to Quizmaster v1.0!");
    mvwprintw(win, 7, 5, "1. Start");
    mvwprintw(win, 8, 5, "2. Leave");
    
    int selectedOption = START_OPTION;
    int ch;
    
    nodelay(stdscr, TRUE);

    while (1) {

        ch = getch();

        switch (ch) {
            case KEY_UP:
                selectedOption = (selectedOption == START_OPTION) ? LEAVE_OPTION : START_OPTION;
                break;
            case KEY_DOWN:
                selectedOption = (selectedOption == LEAVE_OPTION) ? START_OPTION : LEAVE_OPTION;
                break;
            case '\n':
            case KEY_ENTER:
                if (selectedOption == START_OPTION) {
                    werase(win);
                    return;
                    break;
                } else if (selectedOption == LEAVE_OPTION) {
                    endwin();
                    exit(0);
                }
                break;
            default:
                break;
        }

        werase(win);
        mvwprintw(win, 5, 5, "Welcome to Quizmaster v1.0!");
        mvwprintw(win, 7, 5, "1. Start");
        mvwprintw(win, 8, 5, "2. Leave");
        box(win, 0, 0);


        if (selectedOption == START_OPTION) {
            wattron(win, A_REVERSE);
            mvwprintw(win, 7, 5, "1. Start");
            wattroff(win, A_REVERSE);
        } else {
            wattron(win, A_REVERSE);
            mvwprintw(win, 8, 5, "2. Leave");
            wattroff(win, A_REVERSE);
        }

        wrefresh(win);
    }
}