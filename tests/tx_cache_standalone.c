/** (c) 2024 UMSATS
 * @file tx_cache_standalone.c
 *
 * Standalone version of tx_cache.c for testing.
 * Uses mock types instead of real STM32 dependencies.
 */

#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "mock_can_message.h"
#include "mock_tx_cache.h"

static bool is_matching_ack(const CANMessage *msg, const CANMessage *ack);

// Internal validation helper
static inline bool TxCache_IsValid(const TxCache *txc)
{
	// In NDEBUG mode, assertions are removed, so silence unused parameter warning
	(void)txc;
	assert(txc != NULL);
	assert(txc->size <= TX_CACHE_SIZE);
	assert(txc->head < TX_CACHE_SIZE);
	assert(txc->tail < TX_CACHE_SIZE);
	return true;
}

TxCache TxCache_Create(void)
{
	TxCache tx_cache;

	tx_cache.size = 0;
	tx_cache.head = 0;
	tx_cache.tail = 0;

	return tx_cache;
}

bool TxCache_IsFull(const TxCache* txc)
{
    return (txc->tail + 1) % TX_CACHE_SIZE == txc->head;
}

bool TxCache_Push_Back(TxCache *txc, TxCacheItem *item)
{
	assert(txc != NULL);
	assert(item != NULL);
	TxCache_IsValid(txc);

	if (TxCache_IsFull(txc))
		return false;

	txc->items[txc->tail] = *item;
	txc->tail = (txc->tail + 1) % TX_CACHE_SIZE;
	txc->size++;

	TxCache_IsValid(txc);
	return true;
}

int TxCache_Find(const TxCache *txc, const CANMessage *ack)
{
	assert(txc != NULL);
	assert(ack != NULL);
	TxCache_IsValid(txc);

	int index = 0;
	uint32_t i = txc->head;
	while (i != txc->tail && !is_matching_ack(&txc->items[i].msg, ack))
	{
		index++;
		i = (i + 1) % TX_CACHE_SIZE;
	}

	if (i == txc->tail)
		return -1;

	return index;
}

bool TxCache_Erase(TxCache *txc, int index)
{
	assert(txc != NULL);
	TxCache_IsValid(txc);

	if (index < 0 || index >= (int)txc->size) return false;
	if (txc->size == 0) return false; // Defensive check

	// Shift all items after the erased one forward by one position
	for (int i = index; i < (int)txc->size - 1; i++)
	{
		int current_pos = (txc->head + i) % TX_CACHE_SIZE;
		int next_pos = (txc->head + i + 1) % TX_CACHE_SIZE;
		txc->items[current_pos] = txc->items[next_pos];
	}

	// Move tail back by one position
	txc->tail = (txc->tail - 1 + TX_CACHE_SIZE) % TX_CACHE_SIZE;
	txc->size--;

	TxCache_IsValid(txc);
	return true;
}

const TxCacheItem *TxCache_At(const TxCache *txc, int index)
{
	assert(txc != NULL);
	TxCache_IsValid(txc);

	if (index < 0 || index >= (int)txc->size) return NULL;

	// Calculate actual position in circular buffer accounting for head offset
	int pos = (txc->head + index) % TX_CACHE_SIZE;
	return &txc->items[pos];
}

/**
 * @brief Returns true if the ACK message corresponds to msg.
 *
 * A match signifies:
 * 1. the message data and priority fields are identical
 * 2. the sender of msg is the recipient of ack, and,
 * 3. the recipient of ack is the sender of msg.
 *
 * @param msg  the message.
 * @param ack  the acknowledging message.
 * @return     true if the messages match. False otherwise.
 */
static bool is_matching_ack(const CANMessage *msg, const CANMessage *ack)
{
	return !msg->is_ack
		&& ack->is_ack
		&& msg->cmd == ack->cmd
		&& msg->body_size == ack->body_size
		&& memcmp(msg->body, ack->body, msg->body_size) == 0
		&& msg->priority == ack->priority
		&& msg->sender == ack->recipient
		&& msg->recipient == ack->sender;
}
