/** (c) 2024 UMSATS
 * @file mock_tx_cache.h
 *
 * Mock version of tx_cache.h for standalone testing.
 * Uses mock_can_message.h instead of real CAN dependencies.
 */

#ifndef MOCK_TX_CACHE_H_
#define MOCK_TX_CACHE_H_

#include "mock_can_message.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TX_CACHE_SIZE 100

typedef struct
{
	uint32_t timestamp;
	CANMessage msg;
} TxCacheItem;

typedef struct
{
	size_t size;
    uint32_t head;
    uint32_t tail;
    TxCacheItem items[TX_CACHE_SIZE];
} TxCache;

TxCache TxCache_Create(void);

bool TxCache_IsFull(const TxCache* txc);

bool TxCache_Push_Back(TxCache *txc, TxCacheItem *item);

int TxCache_Find(const TxCache *txc, const CANMessage *ack);

bool TxCache_Erase(TxCache *txc, int index);

const TxCacheItem *TxCache_At(const TxCache *txc, int index);

#endif /* MOCK_TX_CACHE_H_ */
