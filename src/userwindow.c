#include "../include/userwindow.h"
#include "../include/db.h"
#include "../include/window.h"
#include <ncurses.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

char *username;

void user_menu() {
  printw("###############################################\n");
  printw("#                 User Menu                  #\n");
  printw("###############################################\n");

  printw("Enter your username: ");

  echo();
  refresh();

  username = (char *)malloc(100);
  if (!username) {
    log_error(MEMORY_ALLOCATION_ERROR, "Failed to allocate memory for username");
    return;
  }

  while (1) {
    getstr(username);
    if (strlen(username) == 0) {
      printw("Username cannot be empty. Please enter your username: ");
      refresh();
    } else {
      break;
    }
  }

  clear_screen();

  typedef struct {
    char *name;
    void (*func)();
  } UserMenu;

  UserMenu user_options[] = {{"Borrow Book", borrow_book_menu},
                             {"Return Book", return_book_menu},
                             {"List Borrowed Books", NULL},
                             {"Search Book by Title", NULL}};

  int highlight = 0;
  int size = sizeof(user_options) / sizeof(user_options[0]);

  while (1) {
    clear_screen();

    printw("Welcome, %s\n", username);

    printw("###############################################\n");
    printw("#                 User Menu                  #\n");
    printw("###############################################\n");

    for (int i = 0; i < size; i++) {
      if (i == highlight)
        attron(A_REVERSE); // Highlight current option
      printw("%d. %s\n", i + 1, user_options[i].name);
      if (i == highlight)
        attroff(A_REVERSE); // Remove highlighting
    }

    printw("\nUse Arrow Keys to navigate, Enter to select, q to quit: ");
    refresh();

    int ch = getch();
    switch (ch) {
    case KEY_UP:
      highlight = (highlight == 0) ? size - 1 : highlight - 1;
      break;
    case KEY_DOWN:
      highlight = (highlight == size - 1) ? 0 : highlight + 1;
      break;
    case '\n': // Enter key
      call_menu(user_options[highlight].func);
      return;
    case 'q': // Quit key
      return;
    default:
      break;
    }
  }

  free(username);
}

void borrow_book_menu() {
  printw("###############################################\n");
  printw("#               Borrow Book                   #\n");
  printw("###############################################\n");

  echo();

  printw("Enter book ID to borrow: ");
  refresh();
  int book_id;
  scanw("%d", &book_id);

  // Open the database
  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  // Check if the book exists
  sqlite3_stmt *stmt;
  char check_sql[200];
  sprintf(check_sql, "SELECT * FROM BOOKS WHERE ID = %d", book_id);
  sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0);

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    // If book is not found
    printw("\nBook not found!\n");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    refresh();
    getch();
    return;
  }

  // Close the statement for reuse
  sqlite3_finalize(stmt);

  // Borrow the book
  int err = borrow_book(db, book_id, username);

  if (err) {
    // If book is already borrowed
    printw("\nBook is already borrowed\n");
    return;
  } else {
    printw("\nBook borrowed successfully!\n");
  }

  // Close the database connection
  sqlite3_close(db);
  // Prompt to continue
  printw("Press any key to return to the menu...\n");
  refresh();
  getch();
}

void return_book_menu() {
  printw("###############################################\n");
  printw("#               Return Book                   #\n");
  printw("###############################################\n");

  echo();

  printw("Enter book ID to return: ");
  refresh();
  int book_id;
  scanw("%d", &book_id);

  // Open the database
  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  // Check if the book exists
  sqlite3_stmt *stmt;
  char check_sql[200];
  sprintf(check_sql, "SELECT * FROM BOOKS WHERE ID = %d", book_id);
  sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0);

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    // If book is not found
    printw("\nBook not found!\n");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    refresh();
    getch();
    return;
  }

  // Close the statement for reuse
  sqlite3_finalize(stmt);

  // Return the book
  return_book(db, book_id);

  // Close the database connection
  sqlite3_close(db);

  // Prompt to continue
  printw("\nBook returned successfully!\n");
  printw("Press any key to return to the menu...\n");
  refresh();
  getch();
}
