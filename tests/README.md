# TxCache Standalone Test Suite

Comprehensive test suite for the `tx_cache` circular buffer implementation, designed to run without STM32 HAL or FreeRTOS dependencies.

## Overview

This test suite validates the correctness of the `tx_cache` module through 15 comprehensive tests covering:

- **Basic Operations**: Cache creation, pushing items, full detection
- **Search Operations**: Finding messages by ACK, ACK matching logic
- **Erase Operations**: Removing items from beginning, middle, end
- **Circular Buffer Behavior**: Wraparound handling, index translation
- **Edge Cases**: Invalid indices, NULL pointers, sequential operations
- **Boundary Conditions**: Full cache, empty cache, capacity limits

## Files

- **`test_tx_cache.c`** - Main test suite (700+ lines, 15 tests, 200+ assertions)
- **`tx_cache_standalone.c`** - Standalone tx_cache implementation (mirrors `Src/can_wrapper/tx_cache.c`)
- **`mock_can_message.h`** - Mock CAN message types (no STM32 dependencies)
- **`mock_tx_cache.h`** - Mock tx_cache header
- **`Makefile`** - Build system with multiple targets

## Building and Running

### Quick Start

```bash
# Build and run all tests
make

# Or explicitly:
make all
```

### Build Targets

```bash
# Run tests without rebuilding
make test

# Build and run with Address Sanitizer (memory safety validation)
make asan

# Build with debug symbols and no optimization
make debug

# Clean build artifacts
make clean

# Show help
make help
```

## Test Output

Successful test run:
```
╔════════════════════════════════════════════════════════════╗
║          TX_CACHE COMPREHENSIVE TEST SUITE                 ║
╚════════════════════════════════════════════════════════════╝

Running test_cache_creation...
  PASS
Running test_push_single_item...
  PASS
...
Running test_sequential_erase...
  PASS

╔════════════════════════════════════════════════════════════╗
║                    TEST SUMMARY                            ║
╠════════════════════════════════════════════════════════════╣
║  Tests Passed:  15                                        ║
║  Tests Failed:  0                                         ║
║  Total Tests:   15                                        ║
╠════════════════════════════════════════════════════════════╣
║  ✓✓✓ ALL TESTS PASSED! ✓✓✓                             ║
╚════════════════════════════════════════════════════════════╝
```

## Test Coverage

### Test 1: Cache Creation
Verifies initial state (size=0, head=0, tail=0, not full).

### Test 2: Push Single Item
Tests adding one item and retrieving it.

### Test 3: Push Multiple Items
Validates adding multiple items in sequence.

### Test 4: Fill Cache to Capacity
Tests behavior when cache reaches maximum capacity (TX_CACHE_SIZE - 1).

### Test 5: Find Message with ACK
Validates ACK matching logic:
- Sender/recipient must be swapped
- Command and data must match
- is_ack flag must be set

### Test 6: Erase from Beginning
Tests removing the first item and verifying remaining items shift correctly.

### Test 7: Erase from Middle
Tests removing a middle item and verifying shift behavior.

### Test 8: Erase from End
Tests removing the last item.

### Test 9: Erase with Invalid Indices
Tests error handling for negative, out-of-bounds, and size-equal indices.

### Test 10: At() with Invalid Indices
Validates NULL returns for invalid indices.

### Test 11: Circular Wraparound
Tests circular buffer behavior:
1. Fill cache to near capacity
2. Erase items from beginning
3. Add new items causing wraparound
4. Verify correct ordering

### Test 12: ACK Matching - Wrong Nodes
Verifies ACKs with incorrect sender/recipient don't match.

### Test 13: ACK Matching - Wrong Command
Verifies ACKs with incorrect command don't match.

### Test 14: NULL Pointer Handling
Tests all functions handle NULL pointers gracefully.

### Test 15: Sequential Erase
Tests erasing all items one by one from the beginning.

## Memory Safety

The test suite can be validated with Address Sanitizer:

```bash
make asan
```

This detects:
- Buffer overruns
- Use-after-free
- Memory leaks
- Invalid memory access

## Integration

### CI/CD Integration

Add to GitHub Actions workflow:

```yaml
- name: Run tx_cache tests
  run: |
    cd tests
    make clean
    make all
```

### Continuous Validation

Run tests before committing changes:

```bash
cd tests
make clean && make asan
```

## Critical Bugs Fixed

This test suite validates fixes for two critical bugs in the original implementation:

### Bug 1: TxCache_Erase - Incorrect Index Translation
**Location**: `Src/can_wrapper/tx_cache.c:57-75`

**Before (BUGGY)**:
```c
while (cur_pos != index && next_pos != txc->tail)  // Comparing position to logical index!
```

**After (FIXED)**:
```c
for (int i = index; i < (int)txc->size - 1; i++)
{
    int current_pos = (txc->head + i) % TX_CACHE_SIZE;  // Correct translation
    int next_pos = (txc->head + i + 1) % TX_CACHE_SIZE;
    txc->items[current_pos] = txc->items[next_pos];
}
```

### Bug 2: TxCache_At - Missing Head Offset
**Location**: `Src/can_wrapper/tx_cache.c:77-84`

**Before (BUGGY)**:
```c
int pos = index % TX_CACHE_SIZE;  // Doesn't account for head offset!
```

**After (FIXED)**:
```c
int pos = (txc->head + index) % TX_CACHE_SIZE;  // Correct position calculation
```

## Development Workflow

### Adding New Tests

1. Add test function following the pattern:
   ```c
   bool test_your_feature(void)
   {
       TxCache cache = TxCache_Create();
       // ... test code ...
       TEST_ASSERT(condition, "description");
       return true;
   }
   ```

2. Register in `main()`:
   ```c
   RUN_TEST(test_your_feature);
   ```

### Syncing with Production Code

When `Src/can_wrapper/tx_cache.c` is updated:

1. Apply same changes to `tests/tx_cache_standalone.c`
2. Run test suite to verify changes
3. Add new tests for new functionality

## Compiler Warnings

The test suite compiles with strict warnings:
- `-Wall` - All common warnings
- `-Wextra` - Extra warnings
- `-Werror` - Treat warnings as errors
- `-Wpedantic` - ISO C compliance

Zero warnings are tolerated.

## Performance

Test execution is fast:
- **Build time**: <1 second on modern hardware
- **Test execution**: <0.1 seconds
- **Total cycle**: ~1 second from `make clean` to results

This enables rapid development iteration without hardware dependencies.

## Thread Safety

The tx_cache module does NOT provide internal synchronization. In production:

- **Cache_Manager_Thread** is the SINGLE OWNER of `s_tx_cache`
- All cache operations must go through the cache command queue
- ISRs and other threads NEVER access the cache directly

This single-owner pattern ensures race-condition-free operation in the FreeRTOS environment.

## License

(c) 2024 UMSATS

## Contact

For questions or issues, contact the UMSATS software team.
