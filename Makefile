
# Makefile for ls utility project
# Makefile for lsv1.1.0
# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Files
SRC = $(SRC_DIR)/lsv1.1.0.c
OBJ = $(OBJ_DIR)/lsv1.1.0.o
BIN = $(BIN_DIR)/ls

# Compiler
CC = gcc
CFLAGS = -Wall -g

# Default Target
all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

$(OBJ): $(SRC)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean

