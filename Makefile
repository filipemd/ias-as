#   Copyright (C) 2025  filipemd
#
#   This file is part of IAS Assembler.
#
#   IAS Assembler is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   IAS Assembler is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with IAS Assembler.  If not, see <http://www.gnu.org/licenses/>.

# Compiler to use
CC = cc

# Compiler flags (e.g., -Wall, -Werror, -g)
CFLAGS = -Wall -Wpedantic -Wextra -O2 -std=gnu99 -g

# Source file(s)
SRCS = $(wildcard src/*.c)

# Object file(s)
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))

# Name of the executable
TARGET = build/ias-as

# Default target
all: $(TARGET)

# Rule to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Rule to create object files from source files
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to create the build directory if it doesn't exist
build:
	mkdir -p build

# Clean rule to remove object files and the executable
clean:
	rm -rf build
