/** (c) 2024 UMSATS
 * @file tx_cache_standalone.c
 *
 * Standalone version of tx_cache.c for testing without STM32 HAL dependencies.
 * This file mirrors the implementation in Src/can_wrapper/tx_cache.c
 */

#include "mock_tx_cache.h"
#include <string.h>
#include <stddef.h>
#include <assert.h>

static bool is_matching_ack(const CANMessage *msg, const CANMessage *ack);

#ifndef NDEBUG
static bool TxCache_IsValid(const TxCache *txc)
{
	if (txc == NULL) return false;
	if (txc->size > TX_CACHE_SIZE) return false;
	if (txc->head >= TX_CACHE_SIZE) return false;
	if (txc->tail >= TX_CACHE_SIZE) return false;
	return true;
}
#else
#define TxCache_IsValid(txc) (true)
#endif

TxCache TxCache_Create(void)
{
	TxCache tx_cache;

	tx_cache.size = 0;
	tx_cache.head = 0;
	tx_cache.tail = 0;

	return tx_cache;
}

bool TxCache_Is_Full(const TxCache* txc)
{
	if (txc == NULL) return true;
	assert(TxCache_IsValid(txc));
	return (txc->tail + 1) % TX_CACHE_SIZE == txc->head;
}

bool TxCache_Push_Back(TxCache *txc, const TxCacheItem *item)
{
	if (txc == NULL || item == NULL) return false;
	assert(TxCache_IsValid(txc));

	if (TxCache_Is_Full(txc))
		return false;

	txc->items[txc->tail] = *item;
	txc->tail = (txc->tail + 1) % TX_CACHE_SIZE;
	txc->size++;

	assert(TxCache_IsValid(txc));
	return true;
}

int TxCache_Find(const TxCache *txc, const CANMessage *ack)
{
	if (txc == NULL || ack == NULL) return -1;
	assert(TxCache_IsValid(txc));

	int index = 0;
	int i = txc->head;
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
	if (txc == NULL) return false;
	assert(TxCache_IsValid(txc));

	if (index < 0 || index >= (int)txc->size) return false;

	// Shift all items after index forward by one position
	for (int i = index; i < (int)txc->size - 1; i++)
	{
		int current_pos = (txc->head + i) % TX_CACHE_SIZE;
		int next_pos = (txc->head + i + 1) % TX_CACHE_SIZE;
		txc->items[current_pos] = txc->items[next_pos];
	}

	// Move tail back and decrease size
	txc->tail = (txc->tail - 1 + TX_CACHE_SIZE) % TX_CACHE_SIZE;
	txc->size--;

	assert(TxCache_IsValid(txc));
	return true;
}

const TxCacheItem *TxCache_At(const TxCache *txc, int index)
{
	if (txc == NULL) return NULL;
	assert(TxCache_IsValid(txc));

	if (index < 0 || index >= (int)txc->size) return NULL;

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
