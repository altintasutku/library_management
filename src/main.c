#include "../include/db.h"
#include "../include/window.h"
#include <ncurses.h>

int main() {
  connect_to_database(DB_FILE);

  start_window();

  return 0;
}
