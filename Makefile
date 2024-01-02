CC = g++
CFLAGS = -Wall -Werror -g
SRC_DIR = src
BUILD_DIR = build
TARGET = example

# List all source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

# Generate a list of object files from the source files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Build target
all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -rf /mnt/fmap/file.txt

