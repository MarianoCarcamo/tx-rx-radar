SRC_DIR = ./src
OUT_DIR = ./build
OBJ_DIR = $(OUT_DIR)/obj
BIN_DIR = $(OUT_DIR)/bin

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o,$(SRC_FILES))

.DEFAULT_GOAL := run

.PHONY: clean run

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OBJ_DIR): $(OUT_DIR)
	mkdir -p $(OBJ_DIR)

$(BIN_DIR): $(OUT_DIR)
	mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c $< -o $@ -lm

$(BIN_DIR)/app.elf: $(OBJ_FILES) | $(BIN_DIR)
	gcc -o $@ $(OBJ_FILES) -lm -ljson-c

all: $(BIN_DIR)/app.elf

clean:
	rm -rf $(OUT_DIR)

run: clean all
	$(BIN_DIR)/app.elf
