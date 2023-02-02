CC=g++
STD=-std=c++17
CF=$(STD)
BUILD_DIR=build

all: $(BUILD_DIR) GenreCounter.out genre.out part.out
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/main.o: main.cpp
	$(CC) $(CF) -c main.cpp -o $(BUILD_DIR)/main.o

GenreCounter.out: $(BUILD_DIR)/main.o
	$(CC) $(CF) -o GenreCounter.out $(BUILD_DIR)/main.o

$(BUILD_DIR)/genre.o: genre.cpp
	$(CC) $(CF) -c genre.cpp -o $(BUILD_DIR)/genre.o

genre.out: $(BUILD_DIR)/genre.o
	$(CC) $(CF) -o genre.out $(BUILD_DIR)/genre.o

$(BUILD_DIR)/part.o: part.cpp
	$(CC) $(CF) -c part.cpp -o $(BUILD_DIR)/part.o

part.out: $(BUILD_DIR)/part.o
	$(CC) $(CF) -o part.out $(BUILD_DIR)/part.o

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) *.o *.out
