/** (c) 2024 UMSATS
 * @file debug_logger.h
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date Aug 6, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_LOGGER_H_
#define TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_LOGGER_H_

#include "tuk/error_list.h"
#include "debug_buffer.h"

#include <stdbool.h>

void DebugLogger_Init();
void DebugLogger_Push_Buffer(DebugBuffer *debug_buffer);
void DebugLogger_Pop_Buffer();

bool DebugLogger_Put(uint8_t byte);

#endif /* TSAT_UTILITIES_KIT_INC_TUK_DEBUG_DEBUG_LOGGER_H_ */
