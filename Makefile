CC = gcc
CFLAGS = -Wall -Wextra -Iinclude $(shell pkg-config --cflags ncursesw)
LIBS = $(shell pkg-config --libs ncursesw)

SRC_DIR = src
OBJ_DIR = obj
BIN = planner

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(BIN): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN)
	rm -rf $(OBJ_DIR)

.PHONY: clean
