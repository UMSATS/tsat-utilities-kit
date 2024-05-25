/**
 * @file tx_cache.h
 * List ADT that caches transmitted CAN messages.
 * Implemented using circular buffer.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date March 6, 2024
 */

#ifndef CAN_WRAPPER_MODULE_INC_TX_CACHE_H_
#define CAN_WRAPPER_MODULE_INC_TX_CACHE_H_

#include "can_message.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TX_CACHE_SIZE 100

typedef struct
{
	struct
	{
		uint32_t counter_value;
		uint32_t rcr_value;
	} timestamp;
	CachedCANMessage msg;
} TxCacheItem;

typedef struct
{
	size_t size;
    uint32_t head;
    uint32_t tail;
    TxCacheItem items[TX_CACHE_SIZE];
} TxCache;

TxCache TxCache_Create();

bool TxCache_IsFull(const TxCache* txc);

bool TxCache_Push_Back(TxCache *txc, TxCacheItem *item);

int TxCache_Find(const TxCache *txc, const CachedCANMessage *ack);

bool TxCache_Erase(TxCache *txc, int index);

const TxCacheItem *TxCache_At(const TxCache *txc, int index);

#endif /* CAN_WRAPPER_MODULE_INC_TX_CACHE_H_ */
