#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "quiz_utility.h"
#include "welcome_screen.h"
#include "score_screen.h"
#include <sqlite3.h>
#include <signal.h>

#define INPUT_BUFFER_SIZE 40

struct ThreadData {
    int countdown;
    char inputBuffer[INPUT_BUFFER_SIZE + 1];
    int correctAnswerGiven;
    pthread_mutex_t countdownMutex;
    pthread_mutex_t inputBufferMutex;
    pthread_mutex_t correctAnswerMutex;
    WINDOW *win;
};

void* countdownThread(void* data) {
    struct ThreadData* threadData = (struct ThreadData*)data;

    while (1) {
        pthread_mutex_lock(&threadData->countdownMutex);

        pthread_mutex_lock(&threadData->correctAnswerMutex);
        if (threadData->correctAnswerGiven) {
            pthread_mutex_unlock(&threadData->correctAnswerMutex);
            pthread_mutex_unlock(&threadData->countdownMutex);
            break;
        }
        pthread_mutex_unlock(&threadData->correctAnswerMutex);

        threadData->countdown--;

        pthread_mutex_unlock(&threadData->countdownMutex);

        napms(1000);

        if (threadData->countdown <= 0) {
            break;
        }
    }

    return NULL;
}

void* userInputThread(void* data) {
    struct ThreadData* threadData = (struct ThreadData*)data;

    nodelay(stdscr, TRUE);

    while (1) {
        int ch = getch();
        if (ch != ERR) {
            pthread_mutex_lock(&threadData->inputBufferMutex);

            switch (ch) {
                case KEY_BACKSPACE:
                case KEY_DC:
                case 127:
                    if (strlen(threadData->inputBuffer) > 0) {
                        wmove(threadData->win, 3, 7);
                        wclrtobot(threadData->win);
                        memset(threadData->inputBuffer, 0, sizeof(threadData->inputBuffer));
                    }
                    break;
                case KEY_DOWN:
                case KEY_UP:
                case KEY_LEFT:
                case KEY_RIGHT:
                    break;
                case '\n':
                case KEY_ENTER:
                    break;
                default:
                    if(strlen(threadData->inputBuffer) <= INPUT_BUFFER_SIZE){
                        strncat(threadData->inputBuffer, (char*)&ch, 1);
                    }
            }

            pthread_mutex_unlock(&threadData->inputBufferMutex);
        }

        pthread_mutex_lock(&threadData->countdownMutex);
        pthread_mutex_lock(&threadData->correctAnswerMutex);

        if (threadData->countdown <= 0 || threadData->correctAnswerGiven == 1) {
            pthread_mutex_unlock(&threadData->correctAnswerMutex);
            pthread_mutex_unlock(&threadData->countdownMutex);
            break;
        }
        pthread_mutex_unlock(&threadData->countdownMutex);
        pthread_mutex_unlock(&threadData->correctAnswerMutex);

        napms(50);
    }

    return NULL;
}

int main() {
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    int terminalHeight, terminalWidth;
    getmaxyx(stdscr, terminalHeight, terminalWidth);
    int windowHeight = 25;
    int windowWidth = 80;
    int windowStartY = (terminalHeight - windowHeight) / 2;
    int windowStartX = (terminalWidth - windowWidth) / 2;
    int centredText = (windowWidth - 15) / 2;
    int correct = 0;

    WINDOW *win = newwin(windowHeight, windowWidth, windowStartY, windowStartX);
    box(win, 0, 0);

    showWelcomePage(win);

    refresh();
    wrefresh(win);

    sqlite3 *db = getDatabaseReference();
    if (db == NULL) {
        fprintf(stderr, "Failed to get the database reference\n");
        return -1;
    }
    int filecontent = parseJsonAndInsertData("../sample.json", db);
    if(filecontent == -1){
        fprintf(stderr,"Parsing Failed\n");
        return -1;
    }

    int totalRows = rowCount(db);

    for(int curQuesId = 1; curQuesId <= totalRows; curQuesId++) {
        struct ThreadData threadData;
        const char* title = getTitleById(db, curQuesId);
        const char* answer = getAnswerById(db, curQuesId);
        int marks = getMarksById(db, curQuesId);
        int timer = getTimeById(db, curQuesId);

        threadData.countdown = timer;
        memset(threadData.inputBuffer, 0, sizeof(threadData.inputBuffer));
        threadData.correctAnswerGiven = 0;
        threadData.win = win;
        pthread_mutex_init(&threadData.countdownMutex, NULL);
        pthread_mutex_init(&threadData.inputBufferMutex, NULL);
        pthread_mutex_init(&threadData.correctAnswerMutex, NULL);

        pthread_t countdownThreadID;
        pthread_create(&countdownThreadID, NULL, countdownThread, (void*)&threadData);

        pthread_t userInputThreadID;
        pthread_create(&userInputThreadID, NULL, userInputThread, (void*)&threadData);

        while (1) {
            box(win, 0, 0);
            mvwprintw(win, 0, windowWidth - 23, "Countdown: %ds", threadData.countdown);
            mvwprintw(win, 0, 12, "Marks: %d", marks);
            mvwprintw(win, 2, 2, "Question: ");
            mvwprintw(win, 2, 12, "%s", title);

            pthread_mutex_lock(&threadData.countdownMutex);
            pthread_mutex_lock(&threadData.inputBufferMutex);
            wmove(win,windowHeight - 5,1);
            whline(win, ACS_HLINE, windowWidth - 2);
            wmove(win,windowHeight - 3,1);
            mvwprintw(win, 22, centredText, "Quizmaster v1.0");



            mvwprintw(win, 3, 2, "Answer: %s", threadData.inputBuffer);

            pthread_mutex_unlock(&threadData.countdownMutex);
            pthread_mutex_unlock(&threadData.inputBufferMutex);

            if (threadData.countdown <= 0 || threadData.correctAnswerGiven) {
                mvwprintw(win, 0, windowWidth - 11, "0s");
                break;
            }

            if (strcmp(threadData.inputBuffer, answer) == 0) {
                mvwprintw(win, 6, 6, "CORRECT");
                correct++;
                pthread_mutex_lock(&threadData.correctAnswerMutex);
                threadData.correctAnswerGiven = 1;
                pthread_mutex_unlock(&threadData.correctAnswerMutex);
            }

            wrefresh(win);
        }

        pthread_join(countdownThreadID, NULL);
        pthread_join(userInputThreadID, NULL);

        pthread_mutex_lock(&threadData.correctAnswerMutex);
        threadData.correctAnswerGiven = 0;
        pthread_mutex_unlock(&threadData.correctAnswerMutex);

        wclrtobot(win);
        pthread_mutex_destroy(&threadData.countdownMutex);
        pthread_mutex_destroy(&threadData.inputBufferMutex);

        if(title != NULL)
            free((void*)title);
        if(answer != NULL)
            free((void*)answer);
    }

    showScorePage(win, correct, totalRows, centredText, windowWidth, windowHeight);
    werase(win);
    destroyDatabaseReference(db);
    endwin();

    return 0;
}