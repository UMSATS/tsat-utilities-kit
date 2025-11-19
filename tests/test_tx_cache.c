/** (c) 2024 UMSATS
 * @file test_tx_cache.c
 * @brief Comprehensive test suite for tx_cache circular buffer implementation
 *
 * Tests cover:
 * - Basic operations (create, push, full detection)
 * - Search operations (find, ACK matching logic)
 * - Erase operations (beginning, middle, end, invalid indices)
 * - Circular buffer wraparound behavior
 * - Edge cases and boundary conditions
 */

#include "mock_tx_cache.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Test framework macros
#define TEST_ASSERT(condition, message) \
	do { \
		if (!(condition)) { \
			printf("  FAIL: %s (line %d)\n", message, __LINE__); \
			return false; \
		} \
	} while(0)

#define RUN_TEST(test_func) \
	do { \
		printf("Running %s...\n", #test_func); \
		if (test_func()) { \
			printf("  PASS\n"); \
			tests_passed++; \
		} else { \
			tests_failed++; \
		} \
		total_tests++; \
	} while(0)

// Test statistics
static int tests_passed = 0;
static int tests_failed = 0;
static int total_tests = 0;

// Helper function to create test items
static TxCacheItem create_test_item(uint32_t timestamp, CmdID cmd, NodeID sender, NodeID recipient)
{
	TxCacheItem item;
	item.timestamp = timestamp;
	item.msg.cmd = cmd;
	item.msg.sender = sender;
	item.msg.recipient = recipient;
	item.msg.priority = 0;
	item.msg.body_size = 0;
	item.msg.is_ack = 0;
	return item;
}

// Helper function to create ACK message
static CANMessage create_ack(CmdID cmd, NodeID sender, NodeID recipient)
{
	CANMessage ack;
	ack.cmd = cmd;
	ack.sender = sender;
	ack.recipient = recipient;
	ack.priority = 0;
	ack.body_size = 0;
	ack.is_ack = 1;
	return ack;
}

// Test 1: Cache creation
bool test_cache_creation(void)
{
	TxCache cache = TxCache_Create();
	TEST_ASSERT(cache.size == 0, "Initial size is 0");
	TEST_ASSERT(cache.head == 0, "Initial head is 0");
	TEST_ASSERT(cache.tail == 0, "Initial tail is 0");
	TEST_ASSERT(!TxCache_Is_Full(&cache), "New cache is not full");
	return true;
}

// Test 2: Push single item
bool test_push_single_item(void)
{
	TxCache cache = TxCache_Create();
	TxCacheItem item = create_test_item(100, 0x10, NODE_CDH, NODE_POWER);

	bool result = TxCache_Push_Back(&cache, &item);
	TEST_ASSERT(result == true, "Push succeeds");
	TEST_ASSERT(cache.size == 1, "Size increases to 1");
	TEST_ASSERT(cache.tail == 1, "Tail moves to 1");
	TEST_ASSERT(cache.head == 0, "Head remains at 0");

	const TxCacheItem *retrieved = TxCache_At(&cache, 0);
	TEST_ASSERT(retrieved != NULL, "Can retrieve item");
	TEST_ASSERT(retrieved->timestamp == 100, "Timestamp matches");
	TEST_ASSERT(retrieved->msg.cmd == 0x10, "Command matches");
	return true;
}

// Test 3: Push multiple items
bool test_push_multiple_items(void)
{
	TxCache cache = TxCache_Create();

	for (int i = 0; i < 5; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		bool result = TxCache_Push_Back(&cache, &item);
		TEST_ASSERT(result == true, "Push succeeds");
	}

	TEST_ASSERT(cache.size == 5, "Size is 5");

	// Verify all items
	for (int i = 0; i < 5; i++) {
		const TxCacheItem *item = TxCache_At(&cache, i);
		TEST_ASSERT(item != NULL, "Can retrieve item");
		TEST_ASSERT(item->timestamp == (uint32_t)(100 + i), "Timestamp matches");
		TEST_ASSERT(item->msg.cmd == (CmdID)(0x10 + i), "Command matches");
	}
	return true;
}

// Test 4: Fill cache to capacity
bool test_fill_cache_to_capacity(void)
{
	TxCache cache = TxCache_Create();

	// TX_CACHE_SIZE is 10, so we can add 9 items before it's full
	for (int i = 0; i < TX_CACHE_SIZE - 1; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		bool result = TxCache_Push_Back(&cache, &item);
		TEST_ASSERT(result == true, "Push succeeds");
	}

	TEST_ASSERT(cache.size == TX_CACHE_SIZE - 1, "Cache is at max capacity");
	TEST_ASSERT(TxCache_Is_Full(&cache), "Cache reports full");

	// Try to add one more - should fail
	TxCacheItem item = create_test_item(999, 0x99, NODE_CDH, NODE_POWER);
	bool result = TxCache_Push_Back(&cache, &item);
	TEST_ASSERT(result == false, "Push fails when full");
	TEST_ASSERT(cache.size == TX_CACHE_SIZE - 1, "Size unchanged");

	return true;
}

// Test 5: Find message with ACK
bool test_find_message_with_ack(void)
{
	TxCache cache = TxCache_Create();

	// Add messages
	TxCacheItem item1 = create_test_item(100, 0x10, NODE_CDH, NODE_POWER);
	TxCacheItem item2 = create_test_item(200, 0x20, NODE_CDH, NODE_ADCS);
	TxCacheItem item3 = create_test_item(300, 0x30, NODE_CDH, NODE_PAYLOAD);

	TxCache_Push_Back(&cache, &item1);
	TxCache_Push_Back(&cache, &item2);
	TxCache_Push_Back(&cache, &item3);

	// Create ACK for item2 (sender and recipient are swapped)
	CANMessage ack = create_ack(0x20, NODE_ADCS, NODE_CDH);

	int index = TxCache_Find(&cache, &ack);
	TEST_ASSERT(index == 1, "Found item at index 1");

	// Try non-existent ACK
	CANMessage bad_ack = create_ack(0x99, NODE_POWER, NODE_CDH);
	index = TxCache_Find(&cache, &bad_ack);
	TEST_ASSERT(index == -1, "Returns -1 for non-existent ACK");

	return true;
}

// Test 6: Erase from beginning
bool test_erase_from_beginning(void)
{
	TxCache cache = TxCache_Create();

	for (int i = 0; i < 5; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		TxCache_Push_Back(&cache, &item);
	}

	bool result = TxCache_Erase(&cache, 0);
	TEST_ASSERT(result == true, "Erase succeeds");
	TEST_ASSERT(cache.size == 4, "Size decreases to 4");

	// Verify remaining items shifted
	const TxCacheItem *item = TxCache_At(&cache, 0);
	TEST_ASSERT(item->msg.cmd == 0x11, "Index 0 is now original index 1");

	item = TxCache_At(&cache, 3);
	TEST_ASSERT(item->msg.cmd == 0x14, "Index 3 is original index 4");

	return true;
}

// Test 7: Erase from middle
bool test_erase_from_middle(void)
{
	TxCache cache = TxCache_Create();

	for (int i = 0; i < 5; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		TxCache_Push_Back(&cache, &item);
	}

	bool result = TxCache_Erase(&cache, 2);
	TEST_ASSERT(result == true, "Erase succeeds");
	TEST_ASSERT(cache.size == 4, "Size decreases to 4");

	// Verify items
	TEST_ASSERT(TxCache_At(&cache, 0)->msg.cmd == 0x10, "Index 0 unchanged");
	TEST_ASSERT(TxCache_At(&cache, 1)->msg.cmd == 0x11, "Index 1 unchanged");
	TEST_ASSERT(TxCache_At(&cache, 2)->msg.cmd == 0x13, "Index 2 is now original index 3");
	TEST_ASSERT(TxCache_At(&cache, 3)->msg.cmd == 0x14, "Index 3 is original index 4");

	return true;
}

// Test 8: Erase from end
bool test_erase_from_end(void)
{
	TxCache cache = TxCache_Create();

	for (int i = 0; i < 5; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		TxCache_Push_Back(&cache, &item);
	}

	bool result = TxCache_Erase(&cache, 4);
	TEST_ASSERT(result == true, "Erase succeeds");
	TEST_ASSERT(cache.size == 4, "Size decreases to 4");

	// Verify remaining items unchanged
	for (int i = 0; i < 4; i++) {
		TEST_ASSERT(TxCache_At(&cache, i)->msg.cmd == (CmdID)(0x10 + i), "Item unchanged");
	}

	return true;
}

// Test 9: Erase with invalid indices
bool test_erase_invalid_indices(void)
{
	TxCache cache = TxCache_Create();

	for (int i = 0; i < 3; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		TxCache_Push_Back(&cache, &item);
	}

	// Test negative index
	bool result = TxCache_Erase(&cache, -1);
	TEST_ASSERT(result == false, "Negative index fails");
	TEST_ASSERT(cache.size == 3, "Size unchanged");

	// Test out of bounds index
	result = TxCache_Erase(&cache, 10);
	TEST_ASSERT(result == false, "Out of bounds index fails");
	TEST_ASSERT(cache.size == 3, "Size unchanged");

	// Test index equal to size
	result = TxCache_Erase(&cache, 3);
	TEST_ASSERT(result == false, "Index equal to size fails");
	TEST_ASSERT(cache.size == 3, "Size unchanged");

	return true;
}

// Test 10: At() with invalid indices
bool test_at_invalid_indices(void)
{
	TxCache cache = TxCache_Create();

	for (int i = 0; i < 3; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		TxCache_Push_Back(&cache, &item);
	}

	// Test negative index
	const TxCacheItem *item = TxCache_At(&cache, -1);
	TEST_ASSERT(item == NULL, "Negative index returns NULL");

	// Test out of bounds index
	item = TxCache_At(&cache, 10);
	TEST_ASSERT(item == NULL, "Out of bounds index returns NULL");

	// Test index equal to size
	item = TxCache_At(&cache, 3);
	TEST_ASSERT(item == NULL, "Index equal to size returns NULL");

	return true;
}

// Test 11: Circular buffer wraparound behavior
bool test_circular_wraparound(void)
{
	TxCache cache = TxCache_Create();

	// Fill cache almost to capacity
	for (int i = 0; i < TX_CACHE_SIZE - 1; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		TxCache_Push_Back(&cache, &item);
	}

	// Erase first 5 items
	for (int i = 0; i < 5; i++) {
		TxCache_Erase(&cache, 0);
	}

	TEST_ASSERT(cache.size == TX_CACHE_SIZE - 6, "Size is 4");

	// Now add 5 more items - this should wrap around
	for (int i = 0; i < 5; i++) {
		TxCacheItem item = create_test_item(200 + i, 0x20 + i, NODE_CDH, NODE_POWER);
		bool result = TxCache_Push_Back(&cache, &item);
		TEST_ASSERT(result == true, "Push succeeds during wraparound");
	}

	TEST_ASSERT(cache.size == TX_CACHE_SIZE - 1, "Size is 9 after wraparound");

	// Verify items are in correct order
	for (int i = 0; i < 4; i++) {
		const TxCacheItem *item = TxCache_At(&cache, i);
		TEST_ASSERT(item->msg.cmd == (CmdID)(0x15 + i), "Old items correct");
	}

	for (int i = 0; i < 5; i++) {
		const TxCacheItem *item = TxCache_At(&cache, 4 + i);
		TEST_ASSERT(item->msg.cmd == (CmdID)(0x20 + i), "New items correct");
	}

	return true;
}

// Test 12: ACK matching logic - wrong sender/recipient
bool test_ack_matching_wrong_nodes(void)
{
	TxCache cache = TxCache_Create();

	TxCacheItem item = create_test_item(100, 0x10, NODE_CDH, NODE_POWER);
	TxCache_Push_Back(&cache, &item);

	// ACK with correct cmd but wrong nodes
	CANMessage ack = create_ack(0x10, NODE_CDH, NODE_POWER); // Not swapped!

	int index = TxCache_Find(&cache, &ack);
	TEST_ASSERT(index == -1, "Wrong sender/recipient doesn't match");

	return true;
}

// Test 13: ACK matching logic - wrong command
bool test_ack_matching_wrong_command(void)
{
	TxCache cache = TxCache_Create();

	TxCacheItem item = create_test_item(100, 0x10, NODE_CDH, NODE_POWER);
	TxCache_Push_Back(&cache, &item);

	// ACK with correct nodes but wrong command
	CANMessage ack = create_ack(0x99, NODE_POWER, NODE_CDH);

	int index = TxCache_Find(&cache, &ack);
	TEST_ASSERT(index == -1, "Wrong command doesn't match");

	return true;
}

// Test 14: NULL pointer handling
bool test_null_pointer_handling(void)
{
	TxCache cache = TxCache_Create();
	TxCacheItem item = create_test_item(100, 0x10, NODE_CDH, NODE_POWER);
	CANMessage ack = create_ack(0x10, NODE_POWER, NODE_CDH);

	// Test Push_Back with NULL
	bool result = TxCache_Push_Back(NULL, &item);
	TEST_ASSERT(result == false, "Push_Back with NULL cache fails");

	result = TxCache_Push_Back(&cache, NULL);
	TEST_ASSERT(result == false, "Push_Back with NULL item fails");

	// Test Find with NULL
	int index = TxCache_Find(NULL, &ack);
	TEST_ASSERT(index == -1, "Find with NULL cache returns -1");

	index = TxCache_Find(&cache, NULL);
	TEST_ASSERT(index == -1, "Find with NULL ack returns -1");

	// Test Erase with NULL
	result = TxCache_Erase(NULL, 0);
	TEST_ASSERT(result == false, "Erase with NULL cache fails");

	// Test At with NULL
	const TxCacheItem *ptr = TxCache_At(NULL, 0);
	TEST_ASSERT(ptr == NULL, "At with NULL cache returns NULL");

	// Test Is_Full with NULL
	bool is_full = TxCache_Is_Full(NULL);
	TEST_ASSERT(is_full == true, "Is_Full with NULL cache returns true");

	return true;
}

// Test 15: Sequential erase operations
bool test_sequential_erase(void)
{
	TxCache cache = TxCache_Create();

	for (int i = 0; i < 5; i++) {
		TxCacheItem item = create_test_item(100 + i, 0x10 + i, NODE_CDH, NODE_POWER);
		TxCache_Push_Back(&cache, &item);
	}

	// Erase all from beginning
	for (int i = 0; i < 5; i++) {
		bool result = TxCache_Erase(&cache, 0);
		TEST_ASSERT(result == true, "Erase succeeds");
		TEST_ASSERT(cache.size == (uint32_t)(4 - i), "Size decreases correctly");
	}

	TEST_ASSERT(cache.size == 0, "Cache is empty");

	// Try to erase from empty cache
	bool result = TxCache_Erase(&cache, 0);
	TEST_ASSERT(result == false, "Erase from empty cache fails");

	return true;
}

// Main test runner
int main(void)
{
	printf("\n");
	printf("╔════════════════════════════════════════════════════════════╗\n");
	printf("║          TX_CACHE COMPREHENSIVE TEST SUITE                 ║\n");
	printf("╚════════════════════════════════════════════════════════════╝\n");
	printf("\n");

	RUN_TEST(test_cache_creation);
	RUN_TEST(test_push_single_item);
	RUN_TEST(test_push_multiple_items);
	RUN_TEST(test_fill_cache_to_capacity);
	RUN_TEST(test_find_message_with_ack);
	RUN_TEST(test_erase_from_beginning);
	RUN_TEST(test_erase_from_middle);
	RUN_TEST(test_erase_from_end);
	RUN_TEST(test_erase_invalid_indices);
	RUN_TEST(test_at_invalid_indices);
	RUN_TEST(test_circular_wraparound);
	RUN_TEST(test_ack_matching_wrong_nodes);
	RUN_TEST(test_ack_matching_wrong_command);
	RUN_TEST(test_null_pointer_handling);
	RUN_TEST(test_sequential_erase);

	printf("\n");
	printf("╔════════════════════════════════════════════════════════════╗\n");
	printf("║                    TEST SUMMARY                            ║\n");
	printf("╠════════════════════════════════════════════════════════════╣\n");
	printf("║  Tests Passed:  %-2d                                        ║\n", tests_passed);
	printf("║  Tests Failed:  %-2d                                        ║\n", tests_failed);
	printf("║  Total Tests:   %-2d                                        ║\n", total_tests);
	printf("╠════════════════════════════════════════════════════════════╣\n");

	if (tests_failed == 0) {
		printf("║  ✓✓✓ ALL TESTS PASSED! ✓✓✓                             ║\n");
	} else {
		printf("║  ✗✗✗ SOME TESTS FAILED ✗✗✗                             ║\n");
	}

	printf("╚════════════════════════════════════════════════════════════╝\n");
	printf("\n");

	return (tests_failed == 0) ? 0 : 1;
}
