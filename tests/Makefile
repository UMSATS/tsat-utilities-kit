# Makefile for tx_cache standalone tests
# (c) 2024 UMSATS

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -pedantic
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer

TARGET = test_tx_cache
TARGET_ASAN = test_tx_cache_asan

SRC = test_tx_cache.c tx_cache_standalone.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean test debug asan

# Default target: build and run tests
all: $(TARGET)
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  Building and running tests..."
	@echo "═══════════════════════════════════════════════════════════"
	@./$(TARGET)

# Build test executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Build with Address Sanitizer
asan: $(SRC)
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $(TARGET_ASAN) $^
	@echo ""
	@echo "═══════════════════════════════════════════════════════════"
	@echo "  Running tests with Address Sanitizer..."
	@echo "═══════════════════════════════════════════════════════════"
	@./$(TARGET_ASAN)

# Just run tests without rebuilding
test: $(TARGET)
	@./$(TARGET)

# Build with debug symbols
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)
	@echo "Debug build complete"

# Clean build artifacts
clean:
	rm -f $(TARGET) $(TARGET_ASAN) $(OBJ) *.o

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build and run tests (default)"
	@echo "  asan     - Build and run tests with Address Sanitizer"
	@echo "  test     - Run tests without rebuilding"
	@echo "  debug    - Build with debug symbols and no optimization"
	@echo "  clean    - Remove all build artifacts"
	@echo "  help     - Show this help message"
