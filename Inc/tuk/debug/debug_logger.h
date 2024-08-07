/** (c) 2024 UMSATS
 * @file debug_logger.h
 *
 * Helper module for writing diagnostic data to a buffer.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date Aug 6, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_LOGGER_H_
#define TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_LOGGER_H_

#include "tuk/debug/log_buffer.h"

#include <stdbool.h>

/**
 * @brief	Initialises the logger with no active buffer.
 *
 */
void DebugLogger_Init();

/**
 * @brief			Sets the newly active buffer.
 *
 * @param buffer	the buffer to place at the top of the buffer stack.
 */
void DebugLogger_Push_Buffer(LogBuffer *buffer);

/**
 * @brief			Pops the active buffer off the stack.
 *
 * The buffer below it becomes the active buffer.
 */
void DebugLogger_Pop_Buffer();

/**
 * @brief			Appends a byte of data to the active buffer.
 *
 * @param byte		the data to append.
 * @return			true on success, false on failure.
 */
bool DebugLogger_Put(uint8_t byte);

#endif /* TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_LOGGER_H_ */
