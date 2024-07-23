CC = gcc
TARGET = reed

# Compilation flags
FLAGS = -O2 -flto #-fsanitize=undefined #-pg

# Define the source files and the object files
SRC = $(wildcard src/*.c src/*/*.c)
OBJ = $(patsubst %.c, build/%.o, $(SRC))

# Default rule to build the program
all: $(TARGET)

# Rule to link the object files and create the executable
$(TARGET): $(OBJ)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJ)

# Rule to compile the source files into object files
build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

# Rule to clean up files
clean:
	rm -f $(TARGET) $(OBJ)
	rm -rf build
	rm -f errorRec.bin fixed.bin corrupted.bin original.bin

# PHONY targets to avoid conflicts with files named 'all' or 'clean'
.PHONY: all clean
