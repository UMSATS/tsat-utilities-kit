/**
 * @file error_buffer.c
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date May 27, 2024
 */

#include <stdbool.h>

#include "tuk/error_tracker/error_buffer.h"

bool ErrorBuffer_Has_Error(ErrorBuffer *error_buffer)
{
	return error_buffer->size > 0;
}

void ErrorBuffer_Clear(ErrorBuffer *error_buffer)
{
	error_buffer->size = 0;
}
