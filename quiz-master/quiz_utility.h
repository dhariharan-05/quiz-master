// quiz_utility.h
#ifndef QUIZ_UTILITY_H
#define QUIZ_UTILITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>

void createQuestionsTable(sqlite3 *db);

void clearQuestionsTable(sqlite3 *db);

int rowCount(sqlite3 *db);

const char* getTitleById(sqlite3 *db, int id);

const char* getAnswerById(sqlite3 *db, int id);

int getMarksById(sqlite3 *db, int id);

int getTimeById(sqlite3 *db, int id);

void displayDatabaseContents(sqlite3 *db);

sqlite3* getDatabaseReference();

int parseJsonAndInsertData(char *file_path,sqlite3 *db);

void destroyDatabaseReference(sqlite3 *db);

#endif 
