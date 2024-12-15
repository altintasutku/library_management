#include "../include/bookwindow.h"
#include "../include/db.h"
#include "../include/window.h"
#include <ncurses.h>
#include <string.h>

void book_menu() {
  typedef struct {
    char *name;
    void (*func)();
  } BookMenu;

  BookMenu books[] = {{"Add Book", add_book},
                      {"List Books", list_books},
                      {"Find Book by ID", book_details},
                      {"Update Book", update_book},
                      {"Search Book by Title", search_book}};

  int size = sizeof(books) / sizeof(books[0]);
  int highlight = 0;

  while (1) {
    clear_screen();

    printw("###############################################\n");
    printw("#                 Books Menu                 #\n");
    printw("###############################################\n");

    for (int i = 0; i < size; i++) {
      if (i == highlight)
        attron(A_REVERSE); // Highlight current option
      printw("%d. %s\n", i + 1, books[i].name);
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
      call_menu(books[highlight].func);
      return;
    case 'q': // Quit key
      return;
    default:
      break;
    }
  }
}

void search_book() {
  // Header with a border
  printw("###############################################\n");
  printw("#             Search Book by Title           #\n");
  printw("###############################################\n");

  echo();

  // Prompt for book title to search
  printw("Enter book title to search: ");
  refresh();
  char title[100];
  getstr(title);

  // Open the database
  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  // Check if the book exists
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

  // Book details output
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char *title = sqlite3_column_text(stmt, 0);
    const unsigned char *author = sqlite3_column_text(stmt, 1);
    const unsigned char *publisher = sqlite3_column_text(stmt, 2);
    int year = sqlite3_column_int(stmt, 3);
    const unsigned char *borrower = sqlite3_column_text(stmt, 4);

    // Display book details with a border and alignment
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

  // Close the database connection
  sqlite3_finalize(stmt);
  sqlite3_close(db);

  // Prompt to continue
  printw("\nPress any key to return to the menu...\n");
  refresh();
  getch();
}

void update_book() {
  // Header with a border
  printw("###############################################\n");
  printw("#             Update Book Info               #\n");
  printw("###############################################\n");

  echo();

  // Prompt for book ID to update
  printw("Enter book ID to update: ");
  refresh();
  int id;
  scanw("%d", &id);

  // Open the database
  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  // Check if the book exists
  sqlite3_stmt *stmt;
  char check_sql[200];
  sprintf(check_sql, "SELECT * FROM BOOKS WHERE ID = %d", id);
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

  // Prompt for new details
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

  // Update the book in the database
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
    // Display SQL error message
    printw("\n###############################################\n");
    printw("#               SQL Error                     #\n");
    printw("###############################################\n");
    printw("Error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    // Confirmation message
    printw("\n###############################################\n");
    printw("#        Book Updated Successfully           #\n");
    printw("###############################################\n");
  }

  // Close the database
  sqlite3_close(db);

  // Prompt to continue
  printw("\nPress any key to return to the menu...\n");
  refresh();
  getch();
}

void book_details(int *id) {

  if (id == NULL) {
    // Header with a border
    printw("###############################################\n");
    printw("#            Book Details                    #\n");
    printw("###############################################\n");

    echo();

    // Prompt for book ID
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

  // Book details output
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char *title = sqlite3_column_text(stmt, 0);
    const unsigned char *author = sqlite3_column_text(stmt, 1);
    const unsigned char *publisher = sqlite3_column_text(stmt, 2);
    int year = sqlite3_column_int(stmt, 3);
    const unsigned char *borrower = sqlite3_column_text(stmt, 4);

    // Display book details with a border and alignment
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
    // If no book is found
    printw("\nBook not found\n");
  }

  printw("\nPress any key to continue...\n");

  // Close the database connection
  sqlite3_finalize(stmt);
  sqlite3_close(db);

  refresh();
  getch();
}

void list_books() {
  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  sqlite3_stmt *stmt;
  // sqlite3_prepare_v2(db, "SELECT * FROM BOOKS", -1, &stmt, 0);
  sqlite3_prepare_v2(
      db,
      "SELECT BOOKS.ID, BOOKS.TITLE, BOOKS.AUTHOR, BOOKS.PUBLISHER, "
      "BOOKS.YEAR, "
      "LOANS.BORROWER_NAME "
      "FROM BOOKS "
      "LEFT JOIN LOANS ON BOOKS.ID = LOANS.BOOK_ID AND LOANS.RETURN_DATE IS "
      "NULL;",
      -1, &stmt, 0);

  Book books[100]; // Adjust size based on expected rows
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

  // Cursor tracking
  int current_row = 0;

  while (1) {
    clear();

    // Header with a border
    printw("###############################################\n");
    printw("#               Book List                    #\n");
    printw("###############################################\n");

    // Column headers with better alignment
    printw("%-5s %-30s %-30s %-20s %-10s %-20s\n", "ID", "Title", "Author",
           "Publisher", "Year", "Borrower");
    printw(
        "---------------------------------------------------------------------"
        "---------------------------------------------------------------------"
        "\n");

    // Display books with highlighting for the current row
    for (int i = 0; i < num_books; i++) {
      if (i == current_row) {
        attron(A_REVERSE); // Highlight the selected row
      }

      mvprintw(5 + i, 0, "%-5d %-30s %-30s %-20s %-10d %-20s", books[i].id,
               books[i].title, books[i].author, books[i].publisher,
               books[i].year,
               strcmp(books[i].borrower, "(null)") != 0 ? books[i].borrower
                                                        : "Not "
                                                          "Borrowed");

      if (i == current_row) {
        attroff(A_REVERSE); // Remove highlighting
      }
    }

    // Footer with instructions
    mvprintw(6 + num_books, 0, "Use UP/DOWN arrows to navigate, Q to quit.");

    // Refresh the screen
    refresh();

    // Handle user input
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
  // Header with a border
  printw("###############################################\n");
  printw("#                 Add Book                   #\n");
  printw("###############################################\n");

  // Prompt for book details with spacing for clarity
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

  // Opening the database
  sqlite3 *db;
  sqlite3_open(DB_FILE, &db);

  // SQL query for inserting the book
  char sql[200];
  sprintf(sql,
          "INSERT INTO BOOKS (TITLE, AUTHOR, PUBLISHER, YEAR) VALUES ('%s', "
          "'%s', '%s', %d);",
          title, author, publisher, year);
  char *zErrMsg = 0;

  // Execute the SQL query
  int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    // Display error message
    printw("\n###############################################\n");
    printw("#               SQL Error                     #\n");
    printw("###############################################\n");
    printw("Error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    // Confirmation message
    printw("\n###############################################\n");
    printw("#          Book Added Successfully           #\n");
    printw("###############################################\n");
  }

  // Close the database
  sqlite3_close(db);

  // Prompt to continue
  printw("\nPress any key to return to the menu...\n");
  refresh();
  getch();
}
