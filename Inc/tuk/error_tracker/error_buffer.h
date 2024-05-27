/**
 * @file error_buffer.h
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date May 27, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_BUFFER_H_
#define TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_BUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ERROR_BUFFER_CAPACITY 7

typedef struct
{
	uint8_t data[ERROR_BUFFER_CAPACITY]; // buffer containing error data.
	size_t size;
} ErrorBuffer;

bool ErrorBuffer_Has_Error(ErrorBuffer *error_buffer);
void ErrorBuffer_Clear(ErrorBuffer *error_buffer);

#endif /* TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_BUFFER_H_ */
