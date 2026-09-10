CC ?= cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Iinclude
TARGET = build/flight_analyzer
SOURCES = src/main.c src/flight_data.c

.PHONY: all run clean sanitize

all: $(TARGET)

$(TARGET): $(SOURCES) include/flight_data.h | build
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET) data/flight_data6.bin

sanitize: | build
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer $(SOURCES) -o build/flight_analyzer_san
	./build/flight_analyzer_san data/flight_data6.bin

clean:
	rm -rf build
