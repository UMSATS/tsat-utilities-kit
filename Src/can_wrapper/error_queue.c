/**
 * @file error_queue.c
 * Queue ADT for storing transmission errors.
 * Implemented using circular buffer.
 *
 * @date August 3, 2024
 */

#include "tuk/can_wrapper/error_queue.h"

ErrorQueue ErrorQueue_Create()
{
	ErrorQueue queue;

    queue.head = 0;
    queue.tail = 0;

    return queue;
}

bool ErrorQueue_IsEmpty(const ErrorQueue* queue)
{
    return queue->head == queue->tail;
}

bool ErrorQueue_IsFull(const ErrorQueue* queue)
{
    return (queue->tail + 1) % ERROR_QUEUE_SIZE == queue->head;
}

bool ErrorQueue_Enqueue(ErrorQueue* queue, CANWrapper_ErrorInfo item)
{
    if (ErrorQueue_IsFull(queue))
        return false;

    queue->items[queue->tail] = item;
    queue->tail = (queue->tail + 1) % ERROR_QUEUE_SIZE;

    return true;
}

bool ErrorQueue_Dequeue(ErrorQueue* queue, CANWrapper_ErrorInfo* out)
{
    if (ErrorQueue_IsEmpty(queue))
        return false;

    *out = queue->items[queue->head];
    queue->head = (queue->head + 1) % ERROR_QUEUE_SIZE;

    return true;
}
