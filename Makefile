CC = gcc
CFLAGS = -Iinclude -Wall -Wextra
LDFLAGS = -lsqlite3 -lncurses
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
TARGET = build/library_manager

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean

