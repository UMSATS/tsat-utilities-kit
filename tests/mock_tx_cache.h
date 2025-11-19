/** (c) 2024 UMSATS
 * @file mock_tx_cache.h
 * @brief Mock tx_cache header for standalone testing
 */

#ifndef MOCK_TX_CACHE_H_
#define MOCK_TX_CACHE_H_

#include "mock_can_message.h"
#include <stdint.h>
#include <stdbool.h>

#define TX_CACHE_SIZE 10

typedef struct {
	CANMessage msg;
	uint32_t timestamp;
} TxCacheItem;

typedef struct {
	TxCacheItem items[TX_CACHE_SIZE];
	int head;
	int tail;
	uint32_t size;
} TxCache;

// Function declarations
TxCache TxCache_Create(void);
bool TxCache_Is_Full(const TxCache *txc);
bool TxCache_Push_Back(TxCache *txc, const TxCacheItem *item);
int TxCache_Find(const TxCache *txc, const CANMessage *msg);
bool TxCache_Erase(TxCache *txc, int index);
const TxCacheItem *TxCache_At(const TxCache *txc, int index);

#endif /* MOCK_TX_CACHE_H_ */
