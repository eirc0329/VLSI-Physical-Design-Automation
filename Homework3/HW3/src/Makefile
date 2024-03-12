CC = g++
CFLAGS = -O3 -std=c++11
TARGET = hw3
SRC_DIR = .
BIN_DIR = ../bin

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(SRC_DIR)/main.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(BIN_DIR)/$(TARGET)

run: $(BIN_DIR)/$(TARGET)
	./$(BIN_DIR)/$(TARGET) $(INPUT_FILE) $(OUTPUT_FILE)
