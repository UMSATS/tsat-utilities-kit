/** (c) 2024 UMSATS
 * @file debug_buffer.h
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date Aug 6, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_BUFFER_H_
#define TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_BUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define DEBUG_BUFFER_MAX 5

typedef struct
{
	uint8_t data[DEBUG_BUFFER_MAX];
	size_t size;
} DebugBuffer;

bool DebugBuffer_IsEmpty(const DebugBuffer *debug_buffer);
void DebugBuffer_Clear(DebugBuffer *debug_buffer);

#endif /* TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_BUFFER_H_ */
