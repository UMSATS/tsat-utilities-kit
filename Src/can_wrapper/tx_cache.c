/**
 * @file tx_cache.h
 * List ADT that caches transmitted CAN messages.
 * Implemented using circular buffer.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date March 6, 2024
 */

#include "tx_cache.h"
#include <stdbool.h>

// TODO: Refactor
/**
 * @brief Returns true if the ACK message corresponds to msg.
 *
 * A match signifies:
 * 1. the message data and priority fields are identical,
 * 2. the sender of msg is the recipient of ack, and
 * 3. the recipient of ack is the sender of msg.
 *
 * @param msg  the message.
 * @param ack  the acknowledging message.
 * @return     true if the messages match. False otherwise.
 */
static bool is_matching_ack(const CachedCANMessage *msg, const CachedCANMessage *ack);

TxCache TxCache_Create()
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
	if (TxCache_IsFull(txc))
		return false;

	txc->items[txc->tail] = *item;
	txc->tail = (txc->tail + 1) % TX_CACHE_SIZE;
	txc->size++;

	return true;
}

int TxCache_Find(const TxCache *txc, const CachedCANMessage *ack)
{
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
	if (index < 0 || index >= txc->size) return false;

	int cur_pos = txc->head;
	int next_pos = (cur_pos + 1) % TX_CACHE_SIZE;
	while (cur_pos != index && next_pos != txc->tail)
	{
		txc->items[next_pos] = txc->items[cur_pos];
		cur_pos = next_pos;
		next_pos = (next_pos + 1) % TX_CACHE_SIZE;
	}

	txc->head = (txc->head + 1) % TX_CACHE_SIZE;
	txc->size--;

	return true;
}

const TxCacheItem *TxCache_At(const TxCache *txc, int index)
{
	if (index < 0 || index >= txc->size) return NULL;

	int pos = index % TX_CACHE_SIZE;
	return &txc->items[pos];
}

static bool is_matching_ack(const CachedCANMessage *msg, const CachedCANMessage *ack)
{
	return !msg->is_ack
		&& ack->is_ack
		&& CANMessage_Equals(&msg->msg, &ack->msg)
		&& msg->priority == ack->priority
		&& msg->sender == ack->recipient
		&& msg->recipient == ack->sender;
}
