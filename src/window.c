#include "../include/window.h"
#include "../include/db.h"
#include "../include/mainwindow.h"
#include <ncurses.h>

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
