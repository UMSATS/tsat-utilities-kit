/** (c) 2024 UMSATS
 * @file can_queue.h
 *
 * Queue ADT for storing CAN messages.
 * Implemented using circular buffer.
 */

#ifndef CAN_WRAPPER_MODULE_INC_CAN_QUEUE_H_
#define CAN_WRAPPER_MODULE_INC_CAN_QUEUE_H_

#include "can_message.h"
#include "tuk/error_list.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint32_t head;
    uint32_t tail;
    CANMessage *buffer;
    size_t max_size;
} CANQueue;

/**
 * @brief              Initialises an empty message queue.
 *
 * @param queue        The queue to initialise.
 * @param buffer       A buffer for storing messages. Must be multiples of sizeof(CANMessage).
 * @param buffer_size  The size of the buffer in bytes.
 */
ErrorCode CANQueue_Init(CANQueue *queue, CANMessage *buffer, size_t buffer_size);

/**
 * @brief              Returns true if the given queue is empty.
 *
 * @param queue        The CAN message queue.
 * @param result
 */
ErrorCode CANQueue_IsEmpty(const CANQueue* queue, bool *result);

/**
 * @brief              Returns true if the given queue is full.
 *
 * @param queue        The CAN message queue.
 * @param result
 */
ErrorCode CANQueue_IsFull(const CANQueue* queue, bool *result);

/**
 * @brief              Enqueues a message into the given queue.
 * @warning            msg must not point to a location inside the queue buffer.
 *
 * @param queue        The CAN message queue.
 * @param msg          The message to enqueue.
 */
ErrorCode CANQueue_Enqueue(CANQueue* queue, const CANMessage *msg);

/**
 * @brief:             Dequeues a message out of the given queue.
 *
 * @param queue        The CAN message queue.
 * @param out          The output location for CAN message.
 */
ErrorCode CANQueue_Dequeue(CANQueue* queue, CANMessage *out);

#endif /* CAN_WRAPPER_MODULE_INC_CAN_QUEUE_H_ */
