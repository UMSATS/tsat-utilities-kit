# Makefile for TxCache Standalone Tests
# TSAT Utilities Kit - UMSATS 2024

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -pedantic
CFLAGS += -I.
CFLAGS += -DNDEBUG  # Disable assertions in production, remove for debugging

TARGET = test_tx_cache
SRC = test_tx_cache.c tx_cache_standalone.c
HEADERS = mock_can_message.h mock_tx_cache.h

# Default target
all: $(TARGET)
	@echo ""
	@echo "Build successful! Running tests..."
	@echo ""
	@./$(TARGET)

# Build the test executable
$(TARGET): $(SRC) $(HEADERS)
	@echo "Compiling $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Run tests only
test: $(TARGET)
	@echo "Running tests..."
	@./$(TARGET)

# Run with verbose output
verbose: CFLAGS += -DVERBOSE
verbose: clean all

# Run with assertions enabled (debug mode)
debug: CFLAGS := $(filter-out -DNDEBUG,$(CFLAGS))
debug: CFLAGS += -O0 -g3 -DDEBUG
debug: clean all

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -f $(TARGET)
	@rm -f *.o
	@rm -f *.out
	@rm -f core

# Run with valgrind for memory checking (if available)
valgrind: $(TARGET)
	@echo "Running with valgrind..."
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Help target
help:
	@echo "TxCache Standalone Test Suite"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build and run tests (default)"
	@echo "  make test     - Run tests without rebuilding"
	@echo "  make debug    - Build with assertions and debug symbols"
	@echo "  make verbose  - Build with verbose output"
	@echo "  make valgrind - Run with valgrind memory checker"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this help message"

.PHONY: all test verbose debug clean valgrind help
