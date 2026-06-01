CC = gcc
CFLAGS = -Wall -Wextra -Iinclude $(shell pkg-config --libs sdl3)
SRC = $(wildcard src/*.c)
LIBS = $(shell pkg-config --libs sdl3)
OBJ = $(SRC:src/%.c=build/%.o)
TARGET = build/cimageview

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build: 
	mkdir -p build

clean: 
	rm -rf build
