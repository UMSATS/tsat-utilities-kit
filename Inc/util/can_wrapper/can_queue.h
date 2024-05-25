/**
 * @file can_queue.h
 * Queue ADT for storing CAN messages.
 * Implemented using circular buffer.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 * @author Om Sevak <om.sevak@umsats.ca>
 *
 * @date February 12, 2024
 */

#ifndef CAN_WRAPPER_MODULE_INC_CAN_QUEUE_H_
#define CAN_WRAPPER_MODULE_INC_CAN_QUEUE_H_

#include <can_message.h>
#include <stdbool.h>
#include <sys/_stdint.h>

#define CAN_QUEUE_SIZE 100

typedef struct
{
	CachedCANMessage msg;
} CANQueueItem;

typedef struct
{
    uint32_t head;
    uint32_t tail;
    CANQueueItem items[CAN_QUEUE_SIZE];
} CANQueue;

/**
 * @brief               Creates an empty message queue.
 */
CANQueue CANQueue_Create();

/**
 * @brief               Returns true if the given queue is empty.
 */
bool CANQueue_IsEmpty(const CANQueue* queue);

/**
 * @brief               Returns true if the given queue is full.
 */
bool CANQueue_IsFull(const CANQueue* queue);

/**
 * @brief               Enqueues a message into the given queue.
 *
 * @param queue         The CAN message queue.
 * @param message       The CAN message to enqueue.
 * @return              true on success. false on fail.
 */
bool CANQueue_Enqueue(CANQueue* queue, CANQueueItem message);

/**
 * @brief:              Dequeues a message out of the given queue.
 *
 * @param queue         The CAN message queue.
 * @param out_message   The output location for CAN message.
 * @return              true on success. false on fail.
 */
bool CANQueue_Dequeue(CANQueue* queue, CANQueueItem* out_message);

#endif /* CAN_WRAPPER_MODULE_INC_CAN_QUEUE_H_ */
