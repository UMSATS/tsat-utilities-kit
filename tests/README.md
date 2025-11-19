# TxCache Standalone Test Suite

Comprehensive test suite for the TxCache circular buffer implementation.

## Overview

This test suite validates the correctness of `tx_cache.c` without requiring STM32 HAL, FreeRTOS, or any embedded dependencies. Tests can be compiled and run on any platform with a C11-compliant compiler.

## Files

- **test_tx_cache.c** - Main test file with 15 comprehensive tests
- **tx_cache_standalone.c** - Standalone implementation of tx_cache (mirrors Src/can_wrapper/tx_cache.c)
- **mock_can_message.h** - Mock CAN message types
- **mock_tx_cache.h** - Mock tx_cache header
- **Makefile** - Build system
- **README.md** - This file

## Building and Running

### Quick Start
```bash
make          # Build and run all tests
```

### Other Targets
```bash
make test     # Run existing test executable
make debug    # Build with assertions enabled
make clean    # Remove build artifacts
make valgrind # Run with memory leak detection (requires valgrind)
make help     # Show all available targets
```

## Test Coverage

The suite includes 15 tests covering:

1. **Basic Operations**
   - Cache creation
   - Push single/multiple items
   - Fill cache to capacity

2. **Search Operations**
   - Find existing messages
   - Find non-existent messages
   - ACK matching logic

3. **Erase Operations**
   - Erase from beginning/middle/end
   - Erase with invalid indices
   - Erase all items sequentially

4. **Circular Buffer Behavior**
   - Wraparound handling
   - Repeated fill/empty cycles
   - Index translation

5. **Edge Cases**
   - Invalid indices
   - Empty cache operations
   - Full cache operations
   - Boundary conditions

## Test Results

All tests should pass:
```
Tests Passed:  15
Tests Failed:  0
Total Tests:   15
```

## Integration with CI/CD

These tests are designed to run in CI pipelines (GitHub Actions) to verify:
- Code correctness after changes
- No regressions introduced
- Platform-independent behavior

## Debugging Failed Tests

If a test fails:

1. **Build in debug mode:**
   ```bash
   make debug
   ```
   This enables assertions and debug symbols.

2. **Run with valgrind:**
   ```bash
   make valgrind
   ```
   This checks for memory errors.

3. **Check specific test:**
   Look at the test output to see which assertion failed and why.

## Extending Tests

To add new tests:

1. Add a new test function following the pattern:
   ```c
   void test_my_new_test(void)
   {
       TEST_START("My New Test");

       // ... test code ...

       TEST_ASSERT(condition, "Description");

       TEST_END();
   }
   ```

2. Call it from `main()`:
   ```c
   test_my_new_test();
   ```

3. Rebuild and run:
   ```bash
   make
   ```

## Notes

- Tests use mock types to avoid STM32 dependencies
- The standalone implementation (`tx_cache_standalone.c`) should be kept in sync with `Src/can_wrapper/tx_cache.c`
- Assertions are disabled in production builds (`-DNDEBUG`)
- All tests are deterministic and repeatable

## License

MIT License - UMSATS 2024
