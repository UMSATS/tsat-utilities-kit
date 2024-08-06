/**
 * @file error_buffer.c
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date May 27, 2024
 */

#include <stdbool.h>

#include "tuk/debug/debug_buffer.h"

bool DebugBuffer_IsEmpty(const DebugBuffer *debug_buffer)
{
	return debug_buffer->size == 0;
}

void DebugBuffer_Clear(DebugBuffer *debug_buffer)
{
	debug_buffer->size = 0;
}
