/** (c) 2024 UMSATS
 * @file debug_logger.c
 */

#include "tuk/debug/debug_logger.h"
#include "tuk/debug/print.h"

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define BUFFER_STACK_MAX 3

typedef struct
{
	LogBuffer *buffer_stack[BUFFER_STACK_MAX];
	size_t buffer_stack_size;
} DebugLogger;

static DebugLogger s_logger = {0};

#define PRINT_SUBJECT "DebugLogger"

void DebugLogger_Init()
{
	s_logger.buffer_stack_size = 0;
}

void DebugLogger_Push_Buffer(LogBuffer *buffer)
{
	if (s_logger.buffer_stack_size == BUFFER_STACK_MAX)
	{
		PRINT_ERROR("buffer stack overflow.");
		return;
	}

	s_logger.buffer_stack[s_logger.buffer_stack_size] = buffer;
	s_logger.buffer_stack_size++;

	LogBuffer_Clear(buffer);
}

void DebugLogger_Pop_Buffer()
{
	if (s_logger.buffer_stack_size <= 1)
	{
		PRINT_ERROR("buffer stack underflow.");
		return;
	}

	s_logger.buffer_stack_size--;
}

bool DebugLogger_Put(uint8_t byte)
{
	if (s_logger.buffer_stack_size == 0)
	{
		PRINT_ERROR("no active error buffer.");
		return false;
	}

	LogBuffer *buffer = s_logger.buffer_stack[s_logger.buffer_stack_size-1];

	if (buffer->size == LOG_BUFFER_MAX)
	{
		PRINT_WARN("error buffer overflow.");
		return false;
	}

	buffer->data[buffer->size] = byte;
	buffer->size++;

	return true;
}
