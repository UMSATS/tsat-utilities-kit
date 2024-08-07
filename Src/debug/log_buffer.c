/** (c) 2024 UMSATS
 * @file log_buffer.c
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date Aug 6, 2024
 */

#include <stdbool.h>

#include "tuk/debug/log_buffer.h"

bool LogBuffer_IsEmpty(const LogBuffer *log_buffer)
{
	return log_buffer->size == 0;
}

void LogBuffer_Clear(LogBuffer *log_buffer)
{
	log_buffer->size = 0;
}
