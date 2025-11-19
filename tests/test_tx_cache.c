/** (c) 2024 UMSATS
 * @file test_tx_cache.c
 *
 * Comprehensive standalone test suite for tx_cache.c
 * Tests all functionality without requiring STM32 HAL or FreeRTOS.
 *
 * Build with: make
 * Run with: ./test_tx_cache
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mock_can_message.h"
#include "mock_tx_cache.h"

// Test statistics
static int tests_passed = 0;
static int tests_failed = 0;
static int current_test_assertions = 0;

// Test macros
#define TEST_START(name) \
    do { \
        printf("\n=== TEST: %s ===\n", name); \
        current_test_assertions = 0; \
    } while(0)

#define TEST_ASSERT(condition, message) \
    do { \
        current_test_assertions++; \
        if (condition) { \
            printf("  ✓ %s\n", message); \
        } else { \
            printf("  ✗ FAILED: %s\n", message); \
            tests_failed++; \
            return; \
        } \
    } while(0)

#define TEST_END() \
    do { \
        tests_passed++; \
        printf("  PASSED (%d assertions)\n", current_test_assertions); \
    } while(0)

// Helper function to create a test message
CANMessage create_test_message(CmdID cmd, NodeID sender, NodeID recipient, uint8_t priority, bool is_ack)
{
    CANMessage msg = {0};
    msg.cmd = cmd;
    msg.sender = sender;
    msg.recipient = recipient;
    msg.priority = priority;
    msg.is_ack = is_ack;
    msg.body_size = 3;
    msg.body[0] = 0xAA;
    msg.body[1] = 0xBB;
    msg.body[2] = 0xCC;
    return msg;
}

// Helper function to create a test cache item
TxCacheItem create_test_item(uint32_t timestamp, CmdID cmd, NodeID sender, NodeID recipient)
{
    TxCacheItem item;
    item.timestamp = timestamp;
    item.msg = create_test_message(cmd, sender, recipient, 5, false);
    return item;
}

// ============================================================================
// TEST SUITE
// ============================================================================

void test_cache_creation(void)
{
    TEST_START("Cache Creation");

    TxCache cache = TxCache_Create();

    TEST_ASSERT(cache.size == 0, "Initial size is 0");
    TEST_ASSERT(cache.head == 0, "Initial head is 0");
    TEST_ASSERT(cache.tail == 0, "Initial tail is 0");
    TEST_ASSERT(!TxCache_IsFull(&cache), "New cache is not full");

    TEST_END();
}

void test_push_single_item(void)
{
    TEST_START("Push Single Item");

    TxCache cache = TxCache_Create();
    TxCacheItem item = create_test_item(100, 0x10, NODE_CDH, NODE_POWER);

    bool result = TxCache_Push_Back(&cache, &item);

    TEST_ASSERT(result == true, "Push returns true");
    TEST_ASSERT(cache.size == 1, "Size is 1 after push");
    TEST_ASSERT(cache.head == 0, "Head remains at 0");
    TEST_ASSERT(cache.tail == 1, "Tail advances to 1");

    const TxCacheItem *retrieved = TxCache_At(&cache, 0);
    TEST_ASSERT(retrieved != NULL, "Can retrieve item at index 0");
    TEST_ASSERT(retrieved->timestamp == 100, "Timestamp matches");
    TEST_ASSERT(retrieved->msg.cmd == 0x10, "Command matches");

    TEST_END();
}

void test_push_multiple_items(void)
{
    TEST_START("Push Multiple Items");

    TxCache cache = TxCache_Create();

    for (int i = 0; i < 10; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        bool result = TxCache_Push_Back(&cache, &item);
        TEST_ASSERT(result == true, "Push succeeds for each item");
    }

    TEST_ASSERT(cache.size == 10, "Size is 10 after 10 pushes");
    TEST_ASSERT(cache.head == 0, "Head remains at 0");
    TEST_ASSERT(cache.tail == 10, "Tail is at 10");

    // Verify all items are retrievable and in correct order
    for (int i = 0; i < 10; i++)
    {
        const TxCacheItem *item = TxCache_At(&cache, i);
        TEST_ASSERT(item != NULL, "Item exists at correct index");
        TEST_ASSERT(item->msg.cmd == (CmdID)(0x10 + i), "Item has correct command");
    }

    TEST_END();
}

void test_push_until_full(void)
{
    TEST_START("Push Until Full");

    TxCache cache = TxCache_Create();

    // Fill the cache (TX_CACHE_SIZE - 1 items, because circular buffer leaves one slot)
    for (int i = 0; i < TX_CACHE_SIZE; i++)
    {
        TxCacheItem item = create_test_item(i, 0x10, NODE_CDH, NODE_POWER);
        if (!TxCache_Push_Back(&cache, &item))
        {
            break;
        }
    }

    TEST_ASSERT(TxCache_IsFull(&cache), "Cache reports full");
    TEST_ASSERT(cache.size == TX_CACHE_SIZE - 1, "Size is TX_CACHE_SIZE - 1");

    // Try to push one more - should fail
    TxCacheItem extra = create_test_item(9999, 0xFF, NODE_CDH, NODE_POWER);
    bool result = TxCache_Push_Back(&cache, &extra);
    TEST_ASSERT(result == false, "Push to full cache fails");
    TEST_ASSERT(cache.size == TX_CACHE_SIZE - 1, "Size unchanged after failed push");

    TEST_END();
}

void test_find_existing_message(void)
{
    TEST_START("Find Existing Message");

    TxCache cache = TxCache_Create();

    // Push 5 messages
    for (int i = 0; i < 5; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    // Create matching ACK for message at index 2 (cmd 0x12)
    CANMessage ack = create_test_message(0x12, NODE_POWER, NODE_CDH, 5, true);

    int index = TxCache_Find(&cache, &ack);

    TEST_ASSERT(index == 2, "Found message at correct index");

    TEST_END();
}

void test_find_nonexistent_message(void)
{
    TEST_START("Find Non-existent Message");

    TxCache cache = TxCache_Create();

    // Push 3 messages
    for (int i = 0; i < 3; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    // Create ACK for message that doesn't exist
    CANMessage ack = create_test_message(0xFF, NODE_POWER, NODE_CDH, 5, true);

    int index = TxCache_Find(&cache, &ack);

    TEST_ASSERT(index == -1, "Returns -1 for non-existent message");

    TEST_END();
}

void test_erase_from_beginning(void)
{
    TEST_START("Erase From Beginning");

    TxCache cache = TxCache_Create();

    // Push 5 items
    for (int i = 0; i < 5; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    // Erase first item (index 0)
    bool result = TxCache_Erase(&cache, 0);

    TEST_ASSERT(result == true, "Erase succeeds");
    TEST_ASSERT(cache.size == 4, "Size decreases to 4");

    // Verify remaining items shifted correctly
    const TxCacheItem *first = TxCache_At(&cache, 0);
    TEST_ASSERT(first->msg.cmd == 0x11, "First item is now the original second");

    const TxCacheItem *last = TxCache_At(&cache, 3);
    TEST_ASSERT(last->msg.cmd == 0x14, "Last item unchanged");

    TEST_END();
}

void test_erase_from_middle(void)
{
    TEST_START("Erase From Middle");

    TxCache cache = TxCache_Create();

    // Push 5 items
    for (int i = 0; i < 5; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    // Erase middle item (index 2)
    bool result = TxCache_Erase(&cache, 2);

    TEST_ASSERT(result == true, "Erase succeeds");
    TEST_ASSERT(cache.size == 4, "Size decreases to 4");

    // Verify order
    TEST_ASSERT(TxCache_At(&cache, 0)->msg.cmd == 0x10, "Index 0 unchanged");
    TEST_ASSERT(TxCache_At(&cache, 1)->msg.cmd == 0x11, "Index 1 unchanged");
    TEST_ASSERT(TxCache_At(&cache, 2)->msg.cmd == 0x13, "Index 2 is now original index 3");
    TEST_ASSERT(TxCache_At(&cache, 3)->msg.cmd == 0x14, "Index 3 is now original index 4");

    TEST_END();
}

void test_erase_from_end(void)
{
    TEST_START("Erase From End");

    TxCache cache = TxCache_Create();

    // Push 5 items
    for (int i = 0; i < 5; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    // Erase last item (index 4)
    bool result = TxCache_Erase(&cache, 4);

    TEST_ASSERT(result == true, "Erase succeeds");
    TEST_ASSERT(cache.size == 4, "Size decreases to 4");

    // Verify remaining items unchanged
    for (int i = 0; i < 4; i++)
    {
        TEST_ASSERT(TxCache_At(&cache, i)->msg.cmd == (CmdID)(0x10 + i), "Item unchanged");
    }

    TEST_END();
}

void test_circular_buffer_wraparound(void)
{
    TEST_START("Circular Buffer Wraparound");

    TxCache cache = TxCache_Create();

    // Fill cache almost to capacity
    for (int i = 0; i < TX_CACHE_SIZE - 2; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    // Erase first few items
    TxCache_Erase(&cache, 0);
    TxCache_Erase(&cache, 0);
    TxCache_Erase(&cache, 0);

    size_t size_after_erase = cache.size;

    // Push more items (this should cause wraparound)
    int items_pushed = 0;
    for (int i = 0; i < 5; i++)
    {
        TxCacheItem item = create_test_item(200 + i, 0x20 + i, NODE_POWER, NODE_ADCS);
        if (TxCache_Push_Back(&cache, &item))
        {
            items_pushed++;
        }
    }

    TEST_ASSERT(cache.size == size_after_erase + (size_t)items_pushed, "Size increases correctly after wraparound");

    // Verify we can still access all items
    for (int i = 0; i < (int)cache.size; i++)
    {
        const TxCacheItem *item = TxCache_At(&cache, i);
        TEST_ASSERT(item != NULL, "Can access item after wraparound");
    }

    TEST_END();
}

void test_erase_all_items_sequentially(void)
{
    TEST_START("Erase All Items Sequentially");

    TxCache cache = TxCache_Create();

    // Push 10 items
    const int num_items = 10;
    for (int i = 0; i < num_items; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    // Erase all items from the beginning
    for (int i = 0; i < num_items; i++)
    {
        bool result = TxCache_Erase(&cache, 0);
        TEST_ASSERT(result == true, "Erase succeeds");
        TEST_ASSERT(cache.size == (size_t)(num_items - i - 1), "Size decreases correctly");
    }

    TEST_ASSERT(cache.size == 0, "Cache is empty after erasing all");

    // Try to erase from empty cache
    bool result = TxCache_Erase(&cache, 0);
    TEST_ASSERT(result == false, "Cannot erase from empty cache");

    TEST_END();
}

void test_at_with_invalid_indices(void)
{
    TEST_START("At() With Invalid Indices");

    TxCache cache = TxCache_Create();

    // Push 3 items
    for (int i = 0; i < 3; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    // Test invalid indices
    TEST_ASSERT(TxCache_At(&cache, -1) == NULL, "Negative index returns NULL");
    TEST_ASSERT(TxCache_At(&cache, 3) == NULL, "Index == size returns NULL");
    TEST_ASSERT(TxCache_At(&cache, 100) == NULL, "Large index returns NULL");

    // Test valid indices
    TEST_ASSERT(TxCache_At(&cache, 0) != NULL, "Index 0 is valid");
    TEST_ASSERT(TxCache_At(&cache, 1) != NULL, "Index 1 is valid");
    TEST_ASSERT(TxCache_At(&cache, 2) != NULL, "Index 2 is valid");

    TEST_END();
}

void test_erase_with_invalid_indices(void)
{
    TEST_START("Erase() With Invalid Indices");

    TxCache cache = TxCache_Create();

    // Push 3 items
    for (int i = 0; i < 3; i++)
    {
        TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
        TxCache_Push_Back(&cache, &item);
    }

    size_t original_size = cache.size;

    // Test invalid indices
    TEST_ASSERT(TxCache_Erase(&cache, -1) == false, "Negative index fails");
    TEST_ASSERT(TxCache_Erase(&cache, 3) == false, "Index == size fails");
    TEST_ASSERT(TxCache_Erase(&cache, 100) == false, "Large index fails");

    TEST_ASSERT(cache.size == original_size, "Size unchanged after invalid erases");

    TEST_END();
}

void test_matching_ack_logic(void)
{
    TEST_START("Matching ACK Logic");

    TxCache cache = TxCache_Create();

    // Create original message
    CANMessage original = create_test_message(0x42, NODE_CDH, NODE_POWER, 3, false);
    TxCacheItem item = {.timestamp = 100, .msg = original};
    TxCache_Push_Back(&cache, &item);

    // Create correct ACK (sender and recipient swapped)
    CANMessage correct_ack = create_test_message(0x42, NODE_POWER, NODE_CDH, 3, true);
    int index = TxCache_Find(&cache, &correct_ack);
    TEST_ASSERT(index == 0, "Correct ACK is found");

    // Create ACK with wrong command
    CANMessage wrong_cmd_ack = create_test_message(0x43, NODE_POWER, NODE_CDH, 3, true);
    index = TxCache_Find(&cache, &wrong_cmd_ack);
    TEST_ASSERT(index == -1, "Wrong command ACK not found");

    // Create ACK with wrong priority
    CANMessage wrong_priority_ack = create_test_message(0x42, NODE_POWER, NODE_CDH, 5, true);
    index = TxCache_Find(&cache, &wrong_priority_ack);
    TEST_ASSERT(index == -1, "Wrong priority ACK not found");

    // Create ACK with sender/recipient not swapped
    CANMessage wrong_nodes_ack = create_test_message(0x42, NODE_CDH, NODE_POWER, 3, true);
    index = TxCache_Find(&cache, &wrong_nodes_ack);
    TEST_ASSERT(index == -1, "ACK without swapped nodes not found");

    TEST_END();
}

void test_repeated_fill_empty_cycles(void)
{
    TEST_START("Repeated Fill/Empty Cycles");

    TxCache cache = TxCache_Create();

    // Perform 3 fill/empty cycles
    for (int cycle = 0; cycle < 3; cycle++)
    {
        // Fill
        for (int i = 0; i < 20; i++)
        {
            TxCacheItem item = create_test_item(cycle * 100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
            TxCache_Push_Back(&cache, &item);
        }

        TEST_ASSERT(cache.size == 20, "Filled to 20 items");

        // Empty
        while (cache.size > 0)
        {
            TxCache_Erase(&cache, 0);
        }

        TEST_ASSERT(cache.size == 0, "Emptied completely");
    }

    // Verify cache still works after cycles
    TxCacheItem item = create_test_item(999, 0xFF, NODE_ADCS, NODE_PAYLOAD);
    bool result = TxCache_Push_Back(&cache, &item);
    TEST_ASSERT(result == true, "Can still push after cycles");
    TEST_ASSERT(cache.size == 1, "Size is correct after cycles");

    TEST_END();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║       TxCache Standalone Test Suite                       ║\n");
    printf("║       TSAT Utilities Kit - UMSATS 2024                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    // Run all tests
    test_cache_creation();
    test_push_single_item();
    test_push_multiple_items();
    test_push_until_full();
    test_find_existing_message();
    test_find_nonexistent_message();
    test_erase_from_beginning();
    test_erase_from_middle();
    test_erase_from_end();
    test_circular_buffer_wraparound();
    test_erase_all_items_sequentially();
    test_at_with_invalid_indices();
    test_erase_with_invalid_indices();
    test_matching_ack_logic();
    test_repeated_fill_empty_cycles();

    // Print summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST SUMMARY                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests Passed:  %-3d                                       ║\n", tests_passed);
    printf("║  Tests Failed:  %-3d                                       ║\n", tests_failed);
    printf("║  Total Tests:   %-3d                                       ║\n", tests_passed + tests_failed);
    printf("╠════════════════════════════════════════════════════════════╣\n");

    if (tests_failed == 0)
    {
        printf("║  ✓✓✓ ALL TESTS PASSED! ✓✓✓                             ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
        return 0;
    }
    else
    {
        printf("║  ✗✗✗ SOME TESTS FAILED ✗✗✗                             ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
        return 1;
    }
}
