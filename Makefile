# Makefile - automatic src/*.c -> obj/*.o build for ls project

CC := gcc
CFLAGS := -Wall -Wextra -g

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# find all .c sources in src/
SRCS := $(wildcard $(SRC_DIR)/*.c)

# map src/foo.c -> obj/foo.o  (works even if filenames contain dots)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# output binary name
TARGET := $(BIN_DIR)/ls

.PHONY: all clean run

all: $(TARGET)

# link
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# compile rule for any obj from src
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -rf $(OBJ_DIR)/*.o $(TARGET)

# convenience: run binary (without args)
run: $(TARGET)
	./$(TARGET)


