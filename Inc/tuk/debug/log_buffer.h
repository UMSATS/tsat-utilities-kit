/** (c) 2024 UMSATS
 * @file log_buffer.h
 *
 * A buffer for storing debug data. Used by the Debug Logger.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date Aug 6, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_DEBUG_LOG_BUFFER_H_
#define TSAT_UTILITIES_KIT_INC_TUK_DEBUG_LOG_BUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LOG_BUFFER_MAX 4

typedef struct
{
	uint8_t data[LOG_BUFFER_MAX];
	size_t size;
} LogBuffer;

bool LogBuffer_IsEmpty(const LogBuffer *log_buffer);
void LogBuffer_Clear(LogBuffer *log_buffer);

#endif /* TSAT_UTILITIES_KIT_INC_TUK_DEBUG_LOG_BUFFER_H_ */
