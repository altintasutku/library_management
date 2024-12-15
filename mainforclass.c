/************************************************
 * Library Management System
 * All-in-one source code file for class submission
 ************************************************/

#include <ncurses.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************************************************
 * Constants and Type Definitions
 ************************************************/

#define DB_FILE "library.db"

typedef struct {
  int id;
  char title[100];
  char author[100];
  char publisher[100];
  int year;
  char borrower[100];
} Book;

/************************************************
 * Function Declarations
 ************************************************/

// Database functions
int connect_to_database(const char *db_name);
int create_book_table(sqlite3 *db);
int create_loans_table(sqlite3 *db);
int borrow_book(sqlite3 *db, int book_id, const char *borrower_name);
int return_book(sqlite3 *db, int book_id);

// Window management functions
void start_window();
void call_menu(void *menu);
void clear_screen();

// Main menu functions
void main_menu();

// Book management functions
void book_menu();
void search_book();
void update_book();
void book_details(int *id);
void list_books();
void add_book();

// User management functions
void user_menu();
void borrow_book_menu();
void return_book_menu();

/************************************************
 * Database Management Implementation
 ************************************************/

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

/************************************************
 * Window Management Implementation
 ************************************************/

void clear_screen() {
  clear();
  refresh();
}

void call_menu(void *menu) {
  clear_screen();
  void (*menu_func)() = menu;
  menu_func();
  clear_screen();
}

void start_window() {
  initscr();            // Initialize ncurses
  keypad(stdscr, TRUE); // Enable special keys like arrow keys
  noecho();             // Disable echoing of typed characters
  curs_set(0);          // Hide the cursor

  main_menu();

  endwin(); // End ncurses
}

/************************************************
 * Main Menu Implementation
 ************************************************/

void main_menu() {
  typedef struct {
    char *name;
    void (*func)();
  } MainMenu;

  MainMenu main_menu[] = {
      {"Books", book_menu},
      {"Users", user_menu}
  };

  int size = sizeof(main_menu) / sizeof(main_menu[0]);
  int highlight = 0;

  while (1) {
    clear_screen();
    printw("###############################################\n");
    printw("#              Main Menu                     #\n");
    printw("###############################################\n");

    for (int i = 0; i < size; i++) {
      if (i == highlight)
        attron(A_REVERSE); // Highlight current option
      printw("%d. %s\n", i + 1, main_menu[i].name);
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
      call_menu(main_menu[highlight].func);
      break;
    case 'q': // Quit key
      return;
    default:
      break;
    }
  }
}

/************************************************
 * Book Management Implementation
 ************************************************/

void book_menu() {
  typedef struct {
    char *name;
    void (*func)();
  } BookMenu;

  BookMenu books[] = {
      {"Add Book", add_book},
      {"List Books", list_books},
      {"Find Book by ID", book_details},
      {"Update Book", update_book},
      {"Search Book by Title", search_book}
  };

  int size = sizeof(books) / sizeof(books[0]);
  int highlight = 0;

  while (1) {
    clear_screen();

    printw("###############################################\n");
    printw("#                 Books Menu                 #\n");
    printw("###############################################\n");

    for (int i = 0; i < size; i++) {
      if (i == highlight)
        attron(A_REVERSE);
      printw("%d. %s\n", i + 1, books[i].name);
      if (i == highlight)
        attroff(A_REVERSE);
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
    case '\n':
      call_menu(books[highlight].func);
      return;
    case 'q':
      return;
    default:
      break;
    }
  }
}

void search_book() {
  printw("###############################################\n");
  printw("#             Search Book by Title           #\n");
  printw("###############################################\n");

  echo();

  printw("Enter book title to search: ");
  refresh();
  char title[100];
  getstr(title);

  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  sqlite3_stmt *stmt;
  char check_sql[300];
  snprintf(check_sql, sizeof(check_sql),
           "SELECT BOOKS.TITLE, BOOKS.AUTHOR, BOOKS.PUBLISHER, BOOKS.YEAR, "
           "LOANS.BORROWER_NAME "
           "FROM BOOKS "
           "LEFT JOIN LOANS ON BOOKS.ID = LOANS.BOOK_ID AND LOANS.RETURN_DATE "
           "IS NULL "
           "WHERE BOOKS.TITLE LIKE '%%%s%%';",
           title);
  sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char *title = sqlite3_column_text(stmt, 0);
    const unsigned char *author = sqlite3_column_text(stmt, 1);
    const unsigned char *publisher = sqlite3_column_text(stmt, 2);
    int year = sqlite3_column_int(stmt, 3);
    const unsigned char *borrower = sqlite3_column_text(stmt, 4);

    printw("\n");
    printw("###############################################\n");
    printw("#                 Book Info                  #\n");
    printw("###############################################\n");
    printw("%-20s : %-30s\n", "Title", title);
    printw("%-20s : %-30s\n", "Author", author);
    printw("%-20s : %-30s\n", "Publisher", publisher);
    printw("%-20s : %-10d\n", "Year", year);

    if (borrower != NULL) {
      printw("%-20s : %-30s\n", "Borrowed By", borrower);
    } else {
      printw("%-20s : %-30s\n", "Borrowed By", "Not Borrowed");
    }

    printw("###############################################\n");
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  printw("\nPress any key to return to the menu...\n");
  refresh();
  getch();
}

void update_book() {
  printw("###############################################\n");
  printw("#             Update Book Info               #\n");
  printw("###############################################\n");

  echo();

  printw("Enter book ID to update: ");
  refresh();
  int id;
  scanw("%d", &id);

  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  sqlite3_stmt *stmt;
  char check_sql[200];
  sprintf(check_sql, "SELECT * FROM BOOKS WHERE ID = %d", id);
  sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0);

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    printw("\nBook not found!\n");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    refresh();
    getch();
    return;
  }

  sqlite3_finalize(stmt);

  printw("Enter new book title (leave empty to keep current): ");
  refresh();
  char title[100];
  getstr(title);

  printw("Enter new book author (leave empty to keep current): ");
  refresh();
  char author[100];
  getstr(author);

  printw("Enter new book publisher (leave empty to keep current): ");
  refresh();
  char publisher[100];
  getstr(publisher);

  printw("Enter new book year (leave empty to keep current): ");
  refresh();
  int year;
  scanw("%d", &year);

  noecho();

  char sql[300];
  sprintf(sql,
          "UPDATE BOOKS SET "
          "TITLE = COALESCE(NULLIF('%s', ''), TITLE), "
          "AUTHOR = COALESCE(NULLIF('%s', ''), AUTHOR), "
          "PUBLISHER = COALESCE(NULLIF('%s', ''), PUBLISHER), "
          "YEAR = CASE WHEN %d = 0 THEN YEAR ELSE %d END "
          "WHERE ID = %d;",
          title, author, publisher, year, year, id);

  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

  if (rc != SQLITE_OK) {
    printw("\n###############################################\n");
    printw("#               SQL Error                     #\n");
    printw("###############################################\n");
    printw("Error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    printw("\n###############################################\n");
    printw("#        Book Updated Successfully           #\n");
    printw("###############################################\n");
  }

  sqlite3_close(db);

  printw("\nPress any key to return to the menu...\n");
  refresh();
  getch();
}

void book_details(int *id) {
  if (id == NULL) {
    printw("###############################################\n");
    printw("#            Book Details                    #\n");
    printw("###############################################\n");

    echo();

    printw("Enter book ID: ");
    refresh();
    scanw("%d", id);

    noecho();
  }

  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  sqlite3_stmt *stmt;
  char sql[300];
  snprintf(sql, sizeof(sql),
           "SELECT BOOKS.TITLE, BOOKS.AUTHOR, BOOKS.PUBLISHER, BOOKS.YEAR, "
           "LOANS.BORROWER_NAME "
           "FROM BOOKS "
           "LEFT JOIN LOANS ON BOOKS.ID = LOANS.BOOK_ID AND LOANS.RETURN_DATE "
           "IS NULL "
           "WHERE BOOKS.ID = %d;",
           *id);
  sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char *title = sqlite3_column_text(stmt, 0);
    const unsigned char *author = sqlite3_column_text(stmt, 1);
    const unsigned char *publisher = sqlite3_column_text(stmt, 2);
    int year = sqlite3_column_int(stmt, 3);
    const unsigned char *borrower = sqlite3_column_text(stmt, 4);

    printw("\n");
    printw("###############################################\n");
    printw("#                 Book Info                  #\n");
    printw("###############################################\n");
    printw("%-20s : %-30s\n", "Title", title);
    printw("%-20s : %-30s\n", "Author", author);
    printw("%-20s : %-30s\n", "Publisher", publisher);
    printw("%-20s : %-10d\n", "Year", year);

    if (borrower != NULL) {
      printw("%-20s : %-30s\n", "Borrowed By", borrower);
    } else {
      printw("%-20s : %-30s\n", "Borrowed By", "Not Borrowed");
    }

    printw("###############################################\n");
  } else {
    printw("\nBook not found\n");
  }

  printw("\nPress any key to continue...\n");

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  refresh();
  getch();
}

void list_books() {
  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(
      db,
      "SELECT BOOKS.ID, BOOKS.TITLE, BOOKS.AUTHOR, BOOKS.PUBLISHER, "
      "BOOKS.YEAR, "
      "LOANS.BORROWER_NAME "
      "FROM BOOKS "
      "LEFT JOIN LOANS ON BOOKS.ID = LOANS.BOOK_ID AND LOANS.RETURN_DATE IS "
      "NULL;",
      -1, &stmt, 0);

  Book books[100];
  int num_books = 0;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    books[num_books].id = sqlite3_column_int(stmt, 0);
    snprintf(books[num_books].title, sizeof(books[num_books].title), "%s",
             sqlite3_column_text(stmt, 1));
    snprintf(books[num_books].author, sizeof(books[num_books].author), "%s",
             sqlite3_column_text(stmt, 2));
    snprintf(books[num_books].publisher, sizeof(books[num_books].publisher),
             "%s", sqlite3_column_text(stmt, 3));
    books[num_books].year = sqlite3_column_int(stmt, 4);
    snprintf(books[num_books].borrower, sizeof(books[num_books].borrower), "%s",
             sqlite3_column_text(stmt, 5));
    num_books++;
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  int current_row = 0;

  while (1) {
    clear();

    printw("###############################################\n");
    printw("#               Book List                    #\n");
    printw("###############################################\n");

    printw("%-5s %-30s %-30s %-20s %-10s %-20s\n", "ID", "Title", "Author",
           "Publisher", "Year", "Borrower");
    printw(
        "---------------------------------------------------------------------"
        "---------------------------------------------------------------------"
        "\n");

    for (int i = 0; i < num_books; i++) {
      if (i == current_row) {
        attron(A_REVERSE);
      }

      mvprintw(5 + i, 0, "%-5d %-30s %-30s %-20s %-10d %-20s", books[i].id,
               books[i].title, books[i].author, books[i].publisher,
               books[i].year,
               strcmp(books[i].borrower, "(null)") != 0 ? books[i].borrower
                                                       : "Not Borrowed");

      if (i == current_row) {
        attroff(A_REVERSE);
      }
    }

    mvprintw(6 + num_books, 0, "Use UP/DOWN arrows to navigate, Q to quit.");

    refresh();

    int ch = getch();
    if (ch == KEY_DOWN) {
      if (current_row < num_books - 1) {
        current_row++;
      }
    } else if (ch == KEY_UP) {
      if (current_row > 0) {
        current_row--;
      }
    } else if (ch == '\n') {
      int id = books[current_row].id;
      clear_screen();
      book_details(&id);
    } else if (ch == 'q' || ch == 'Q') {
      break;
    }
  }
}

void add_book() {
  printw("###############################################\n");
  printw("#                 Add Book                   #\n");
  printw("###############################################\n");

  printw("Enter book title: ");
  refresh();
  char title[100];
  getstr(title);

  printw("Enter book author: ");
  refresh();
  char author[100];
  getstr(author);

  printw("Enter book publisher: ");
  refresh();
  char publisher[100];
  getstr(publisher);

  printw("Enter book year: ");
  refresh();
  int year;
  scanw("%d", &year);

  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  char sql[200];
  sprintf(sql,
          "INSERT INTO BOOKS (TITLE, AUTHOR, PUBLISHER, YEAR) VALUES ('%s', "
          "'%s', '%s', %d);",
          title, author, publisher, year);
  char *zErrMsg = 0;

  int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    printw("\n###############################################\n");
    printw("#               SQL Error                     #\n");
    printw("###############################################\n");
    printw("Error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    printw("\n###############################################\n");
    printw("#          Book Added Successfully           #\n");
    printw("###############################################\n");
  }

  sqlite3_close(db);

  printw("\nPress any key to return to the menu...\n");
  refresh();
  getch();
}

/************************************************
 * User Management Implementation
 ************************************************/

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
    printw("Memory allocation error\n");
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

  UserMenu user_options[] = {
      {"Borrow Book", borrow_book_menu},
      {"Return Book", return_book_menu},
      {"List Borrowed Books", NULL},
      {"Search Book by Title", NULL}
  };

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
        attron(A_REVERSE);
      printw("%d. %s\n", i + 1, user_options[i].name);
      if (i == highlight)
        attroff(A_REVERSE);
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
    case '\n':
      call_menu(user_options[highlight].func);
      return;
    case 'q':
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

  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  sqlite3_stmt *stmt;
  char check_sql[200];
  sprintf(check_sql, "SELECT * FROM BOOKS WHERE ID = %d", book_id);
  sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0);

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    printw("\nBook not found!\n");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    refresh();
    getch();
    return;
  }

  sqlite3_finalize(stmt);

  int err = borrow_book(db, book_id, username);

  if (err) {
    printw("\nBook is already borrowed\n");
    return;
  } else {
    printw("\nBook borrowed successfully!\n");
  }

  sqlite3_close(db);
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

  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  sqlite3_stmt *stmt;
  char check_sql[200];
  sprintf(check_sql, "SELECT * FROM BOOKS WHERE ID = %d", book_id);
  sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0);

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    printw("\nBook not found!\n");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    refresh();
    getch();
    return;
  }

  sqlite3_finalize(stmt);

  return_book(db, book_id);

  sqlite3_close(db);
}

/************************************************
 * Main Function
 ************************************************/

int main() {
  connect_to_database(DB_FILE);
  start_window();
  return 0;
} 