/** (c) 2024 UMSATS
 * @file can_queue.c
 *
 * Queue ADT for storing CAN messages.
 * Implemented using circular buffer.
 */

#include "tuk/can_wrapper/can_queue.h"
#include "tuk/error_list.h"
#include "tuk/debug.h"

#include <stdbool.h>
#include <memory.h>

#define BUFFER_MAX 10240 // Anything over 10KB is nonsense.

ErrorCode CANQueue_Init(CANQueue *queue, CANMessage *buffer, size_t buffer_size)
{
	ASSERT_PARAM(queue != NULL, ERR_NULL_ARG);
	ASSERT_PARAM(buffer != NULL, ERR_NULL_ARG);
	ASSERT_PARAM(buffer_size <= BUFFER_MAX, ERR_ARG_OUT_OF_RANGE);
	ASSERT_PARAM(buffer_size % sizeof(CANMessage) == 0, ERR_INVALID_ARG);

	CANQueue q = {0};

	q.head = 0;
	q.tail = 0;
	q.buffer = buffer;
	q.max_size = buffer_size / sizeof(CANMessage);

	*queue = q;

	return ERR_OK;
}

ErrorCode CANQueue_IsEmpty(const CANQueue *queue, bool *result)
{
	ASSERT_PARAM(queue != NULL, ERR_NULL_ARG);
	ASSERT_PARAM(result != NULL, ERR_NULL_ARG);

	*result = queue->head == queue->tail;
	return ERR_OK;
}

ErrorCode CANQueue_IsFull(const CANQueue *queue, bool *result)
{
	ASSERT_PARAM(queue != NULL, ERR_NULL_ARG);
	ASSERT_PARAM(result != NULL, ERR_NULL_ARG);

	*result = (queue->tail + 1) % queue->max_size == queue->head;
	return ERR_OK;
}

ErrorCode CANQueue_Enqueue(CANQueue *queue, const CANMessage *msg)
{
	ASSERT_PARAM(queue != NULL, ERR_NULL_ARG);
	ASSERT_PARAM(msg != NULL, ERR_NULL_ARG);

	bool is_full;
	CANQueue_IsFull(queue, &is_full);
	if (is_full)
		return ERR_QUEUE_FULL;

	memcpy(&queue->buffer[queue->tail], msg, sizeof(CANMessage));
	queue->tail = (queue->tail + 1) % queue->max_size;

	return ERR_OK;
}

ErrorCode CANQueue_Dequeue(CANQueue *queue, CANMessage *out)
{
	ASSERT_PARAM(queue != NULL, ERR_NULL_ARG);
	ASSERT_PARAM(out != NULL, ERR_NULL_ARG);

	bool is_empty;
	CANQueue_IsEmpty(queue, &is_empty);
	if (is_empty)
		return ERR_QUEUE_EMPTY;

	memcpy(out, &queue->buffer[queue->head], sizeof(CANMessage));
	queue->head = (queue->head + 1) % queue->max_size;

	return ERR_OK;
}
