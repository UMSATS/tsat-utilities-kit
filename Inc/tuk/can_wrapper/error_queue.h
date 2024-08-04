/**
 * @file error_queue.h
 * Queue ADT for storing transmission errors.
 * Implemented using circular buffer.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 * @author Om Sevak <om.sevak@umsats.ca>
 * @author Arnav Gupta <arnav.gupta@umsats.ca>
 *
 * @date August 3, 2024
 */

#ifndef CAN_WRAPPER_MODULE_INC_ERROR_QUEUE_H_
#define CAN_WRAPPER_MODULE_INC_ERROR_QUEUE_H_

#include <stdbool.h>
#include <sys/_stdint.h>

#define ERROR_QUEUE_SIZE 100

typedef struct
{
    uint32_t head;
    uint32_t tail;
    CANWrapper_ErrorInfo items[ERROR_QUEUE_SIZE];
} ErrorQueue;

typedef struct
{
	enum
	{
		CAN_WRAPPER_ERROR_TIMEOUT = 0,
		CAN_WRAPPER_ERROR_CAN_TIMEOUT,
	} error;
	union
	{
		struct {
			CANMessage msg;
			NodeID recipient;
		};
		// TODO: more error information.
	};
} CANWrapper_ErrorInfo;

/**
 * @brief               Creates an empty error queue.
 */
ErrorQueue ErrorQueue_Create();

/**
 * @brief               Returns true if the given queue is empty.
 */
bool ErrorQueue_IsEmpty(const ErrorQueue* queue);

/**
 * @brief               Returns true if the given queue is full.
 */
bool ErrorQueue_IsFull(const ErrorQueue* queue);

/**
 * @brief               Enqueues an error into the given queue.
 *
 * @param queue         The error queue.
 * @param error         The error to enqueue.
 * @return              true on success. false on fail.
 */
bool ErrorQueue_Enqueue(ErrorQueue* queue, CANWrapper_ErrorInfo error);

/**
 * @brief:              Dequeues an error out of the given queue.
 *
 * @param queue         The error queue.
 * @param out_error     The output location for the error.
 * @return              true on success. false on fail.
 */
bool ErrorQueue_Dequeue(ErrorQueue* queue, CANWrapper_ErrorInfo* out_error);

#endif /* CAN_WRAPPER_MODULE_INC_ERROR_QUEUE_H_ */
