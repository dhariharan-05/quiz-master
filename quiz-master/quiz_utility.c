#include "quiz_utility.h"

static int rowHelperCallback(void *unused, int count, char **data, char **columns) {
    return 0;
}

void createQuestionsTable(sqlite3 *db) {
    char *createTableSql = "CREATE TABLE IF NOT EXISTS questions (id INTEGER PRIMARY KEY, title TEXT, answer TEXT, marks INTEGER, time INTEGER);";
    int rc = sqlite3_exec(db, createTableSql, rowHelperCallback, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
}

void clearQuestionsTable(sqlite3 *db) {
    char *deleteSql = "DELETE FROM questions;";
    int rc = sqlite3_exec(db, deleteSql, rowHelperCallback, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
}

int rowCount(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM questions;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL parsing error: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    int rows = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        rows = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return rows;
}

const char* getTitleById(sqlite3 *db, int id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT title FROM questions WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL parsing error: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    sqlite3_bind_int(stmt, 1, id);
    const char *title = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        title = strdup((const char *)sqlite3_column_text(stmt, 0));
        if(title == NULL){
            fprintf(stderr, "Memory allocation error \n");
            return NULL;
        }
    }
    sqlite3_finalize(stmt);
    return title;
}

const char* getAnswerById(sqlite3 *db, int id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT answer FROM questions WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL parsing error: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    sqlite3_bind_int(stmt, 1, id);
    const char *answer = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        answer = strdup((const char *)sqlite3_column_text(stmt, 0));
        if(answer == NULL){
            fprintf(stderr, "Memory allocation error \n");
            return NULL;
        }
    }
    sqlite3_finalize(stmt);
    return answer;
}

int getMarksById(sqlite3 *db, int id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT marks FROM questions WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL parsing error: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_int(stmt, 1, id);
    int marks = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        marks = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return marks;
}

int getTimeById(sqlite3 *db, int id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT time FROM questions WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL parsing error: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_int(stmt, 1, id);
    int time = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        time = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return time;
}

void displayDatabaseContents(sqlite3 *db) {
    char *sql = "SELECT * FROM questions;";
    int rc = sqlite3_exec(db, sql, rowHelperCallback, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
}

sqlite3* getDatabaseReference() {
    sqlite3 *db;
    int rc = sqlite3_open("quiz_persistence.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    createQuestionsTable(db);
    clearQuestionsTable(db);
    return db;
}

int parseJsonAndInsertData(char *file_path, sqlite3 *db) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *jsonContent = (char *)malloc(fileSize);
    fread(jsonContent, 1, fileSize, file);
    fclose(file);

    cJSON *root = cJSON_Parse(jsonContent);

    if (root == NULL) {
        const char *errorPtr = cJSON_GetErrorPtr();
        if (errorPtr != NULL) {
            fprintf(stderr, "Error parsing: %s\n", errorPtr);
        }
        cJSON_Delete(root);
        free(jsonContent);
        return -1;
    }

    if (db == NULL) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        cJSON_Delete(root);
        free(jsonContent);
        return -1;
    }

    createQuestionsTable(db);
    clearQuestionsTable(db);

    cJSON *question;
    cJSON_ArrayForEach(question, root) {
        const char *questionKey = question->string;
        cJSON *title = cJSON_GetObjectItemCaseSensitive(question, "title");
        cJSON *answer = cJSON_GetObjectItemCaseSensitive(question, "answer");
        cJSON *marks = cJSON_GetObjectItemCaseSensitive(question, "marks");
        cJSON *time = cJSON_GetObjectItemCaseSensitive(question, "time");

        char insert_sql[256];
        snprintf(insert_sql, sizeof(insert_sql), "INSERT INTO questions (title, answer, marks, time) VALUES ('%s', '%s', %d, %d);",
                 title->valuestring, answer->valuestring, marks->valueint, time->valueint);
        int rc = sqlite3_exec(db, insert_sql, rowHelperCallback, 0, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            cJSON_Delete(root);
            free(jsonContent);
            return -1;
        }
    }

    cJSON_Delete(root);
    free(jsonContent);
    return 0;
}

void destroyDatabaseReference(sqlite3 *db) {
    sqlite3_close(db);
}