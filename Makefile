# Compiler and flags
CC = cc
CFLAGS = -Wall -Wpedantic -Wextra -O2 -std=c99 -g

# re2c input/output
RE2C = re2c
RE2C_FLAGS = -W -b
RE2C_SOURCE = src/determine_token.re2c
RE2C_TARGET = $(RE2C_SOURCE).c

# Sources and objects
SRCS = $(filter-out $(RE2C_TARGET), $(wildcard src/*.c)) $(RE2C_TARGET)
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))

# Target binary
TARGET = build/ias-as

# Default
all: $(TARGET)

# Executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# re2c compilation
$(RE2C_TARGET): $(RE2C_SOURCE)
	$(RE2C) $(RE2C_FLAGS) -o $@ $<

# Object compilation
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Build dir
build:
	mkdir -p build

# Clean
clean:
	rm -rf build $(RE2C_TARGET)
