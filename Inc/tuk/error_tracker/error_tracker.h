/**
 * @file can_wrapper.h
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date Jan 15, 2024
 * @modified May 27, 2024
 */

#ifndef INC_ERROR_CONTEXT_H_
#define INC_ERROR_CONTEXT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "error_list.h"

#define ERROR_BUFFER_CAPACITY 6

typedef struct
{
	uint8_t data[ERROR_BUFFER_CAPACITY]; // buffer containing error data.
	size_t size;
} ErrorBuffer;

void ErrorTracker_Init(ErrorBuffer *init_error_buffer);
void ErrorTracker_Push_Buffer(ErrorBuffer *error_buffer);
void ErrorTracker_Pop_Buffer();

bool ErrorBuffer_Has_Error(ErrorBuffer *error_buffer);
void ErrorBuffer_Clear(ErrorBuffer *error_buffer);

// internal function used by PUT_ERROR macro.
void ErrorTracker_Put_Error_(int n, ...);

#define PUT_ERROR(...)ErrorTracker_Put_Error_(NUM_ARGS(__VA_ARGS__),__VA_ARGS__)

#endif /* INC_ERROR_CONTEXT_H_ */
