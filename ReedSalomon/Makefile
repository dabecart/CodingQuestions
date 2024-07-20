# Define the compiler and the target file
CC = gcc
TARGET = reed

# Define the source files and the object files
SRC = ReedSalomon.c
OBJ = $(SRC:.c=.o)

# Default rule to build the program
all: $(TARGET)

# Rule to link the object files and create the executable
$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

# Rule to compile the source files into object files
%.o: %.c
	$(CC) -c $< -o $@

# Rule to clean up the compiled files
clean:
	rm -f $(TARGET) $(OBJ)
	rm errorRec.bin fixed.bin ../corrupted.bin ../original.bin

# PHONY targets to avoid conflicts with files named 'all' or 'clean'
.PHONY: all clean
