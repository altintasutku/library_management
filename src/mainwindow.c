#include "../include/bookwindow.h"
#include "../include/db.h"
#include "../include/userwindow.h"
#include "../include/window.h"
#include <ncurses.h>

void main_menu() {
  typedef struct {
    char *name;
    void (*func)();
  } MainMenu;

  MainMenu main_menu[] = {{
                              "Books",
                              book_menu,
                          },
                          {"Users", user_menu}};

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
