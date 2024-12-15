#include <sqlite3.h>

#ifndef DB_H
#define DB_H

#define DB_FILE "library.db"

int connect_to_database(const char *db_name);
int create_book_table(sqlite3 *db);
int create_loans_table(sqlite3 *db);
int borrow_book(sqlite3 *db, int book_id, const char *borrower_name);
int return_book(sqlite3 *db, int book_id);

typedef struct {
  int id;
  char title[100];
  char author[100];
  char publisher[100];
  int year;
  char borrower[100];
} Book;

#endif // DB_H
