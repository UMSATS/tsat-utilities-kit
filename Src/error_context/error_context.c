/*
 * error_context.c
 *
 *  Created on: Jan 15, 2024
 *      Author: Logan Furedi
 */

#include "tuk/error_context.h"
#include "tuk/log.h"

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define BUFFER_STACK_CAPACITY 3

typedef struct
{
	ErrorBuffer *buffer_stack[BUFFER_STACK_CAPACITY];
	size_t buffer_stack_size;
} ErrorContext;

static ErrorContext s_context;

static bool push_byte(ErrorBuffer *buffer, uint8_t byte);

#define LOG_SUBJECT "ErrorContext"

void ErrorContext_Init(ErrorBuffer *init_error_buffer)
{
	s_context.buffer_stack_size = 0;

	ErrorContext_Push_Buffer(init_error_buffer);
}

void ErrorContext_Push_Buffer(ErrorBuffer *error_buffer)
{
	if (s_context.buffer_stack_size == BUFFER_STACK_CAPACITY)
	{
		LOG_ERROR("context stack overflow.");
		return;
	}

	s_context.buffer_stack[s_context.buffer_stack_size] = error_buffer;
	s_context.buffer_stack_size++;

	ErrorBuffer_Clear(error_buffer);
}

void ErrorContext_Pop_Buffer()
{
	if (s_context.buffer_stack_size <= 1)
	{
		LOG_ERROR("context stack underflow.");
		return;
	}

	s_context.buffer_stack_size--;
}

void ErrorContext_Push_Error_(int n, ...)
{
	if (s_context.buffer_stack_size == 0)
	{
		LOG_ERROR("no active context.");
		return;
	}

	bool success;

	ErrorBuffer *buffer = s_context.buffer_stack[s_context.buffer_stack_size-1];

	va_list va_ptr;

	va_start(va_ptr, n);

	uint8_t error_code = (uint8_t)va_arg(va_ptr, int);

	success = push_byte(buffer, error_code);

	int i = 0;
	while (success && i < n-1)
	{
		success = push_byte(buffer, (uint8_t)va_arg(va_ptr, int));
		i++;
	}

	va_end(va_ptr);
}

static bool push_byte(ErrorBuffer *buffer, uint8_t byte)
{
	if (buffer->size == ERROR_BUFFER_CAPACITY)
	{
		LOG_WARN("error buffer overflow.");
		return false;
	}

	buffer->data[buffer->size] = byte;
	buffer->size++;

	return true;
}

bool ErrorBuffer_Has_Error(ErrorBuffer *error_buffer)
{
	return error_buffer->size > 0;
}

void ErrorBuffer_Clear(ErrorBuffer *error_buffer)
{
	error_buffer->size = 0;
}
