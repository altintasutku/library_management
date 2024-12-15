#include "../include/db.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

int connect_to_database(const char *db_name) {
  sqlite3 *db;
  int rc = sqlite3_open(db_name, &db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return rc;
  }

  create_book_table(db);
  create_loans_table(db);

  sqlite3_close(db);
  return 0;
}

int create_book_table(sqlite3 *db) {
  char *sql = "CREATE TABLE IF NOT EXISTS BOOKS("
              "ID INTEGER PRIMARY KEY     AUTOINCREMENT,"
              "TITLE           TEXT    NOT NULL,"
              "AUTHOR          TEXT    NOT NULL,"
              "PUBLISHER       TEXT    NOT NULL,"
              "YEAR            INT     NOT NULL);";
  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return rc;
  }
  return 0;
}

int create_loans_table(sqlite3 *db) {
  char *sql = "CREATE TABLE IF NOT EXISTS LOANS("
              "ID INTEGER PRIMARY KEY     AUTOINCREMENT,"
              "BOOK_ID         INT     NOT NULL,"
              "BORROWER_NAME   TEXT    NOT NULL,"
              "BORROW_DATE     TEXT    NOT NULL,"
              "RETURN_DATE     TEXT,"
              "FOREIGN KEY (BOOK_ID) REFERENCES BOOKS(ID));";
  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return rc;
  }
  return 0;
}

int borrow_book(sqlite3 *db, int book_id, const char *borrower_name) {
  // Check if the book is already borrowed
  char sql1[256];
  snprintf(sql1, sizeof(sql1),
           "SELECT * FROM LOANS WHERE BOOK_ID = %d AND RETURN_DATE IS NULL;",
           book_id);

  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, sql1, -1, &stmt, 0);
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    fprintf(stderr, "Book is already borrowed\n");
    return 1;
  }

  // Insert the loan record
  char sql[256];
  snprintf(sql, sizeof(sql),
           "INSERT INTO LOANS (BOOK_ID, BORROWER_NAME, BORROW_DATE) "
           "VALUES (%d, '%s', datetime('now'));",
           book_id, borrower_name);

  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return rc;
  }
  return 0;
}

int return_book(sqlite3 *db, int loan_id) {
  char sql[256];
  snprintf(sql, sizeof(sql), "DELETE FROM LOANS WHERE BOOK_ID = %d;", loan_id);

  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return rc;
  }
  return 0;
}
