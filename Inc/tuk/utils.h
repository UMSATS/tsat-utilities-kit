/**
 * @file utils.h
 * Provides useful miscellaneous utilities for all subsystems.
 *
 * @date Jun 14, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_UTILS_H_
#define TSAT_UTILITIES_KIT_INC_TUK_UTILS_H_

#include <stdint.h>

/**
 * @brief Converts a big-endian (MSB first) buffer of two 8-bit values into a
 *        16-bit value with the processor's native endianness.
 */
static inline uint16_t BE_To_Native_16(const uint8_t *buf)
{
	return (uint16_t)((uint16_t)buf[0] << 8 | (uint16_t)buf[1]);
}

/**
 * @brief Converts a big-endian (MSB first) buffer of four 8-bit values into a
 *        32-bit value with the processor's native endianness.
 */
static inline uint32_t BE_To_Native_32(const uint8_t *buf)
{
	return (uint32_t)buf[0] << 24 |
	       (uint32_t)buf[1] << 16 |
	       (uint32_t)buf[2] << 8 |
	       (uint32_t)buf[3];
}

#endif /* TSAT_UTILITIES_KIT_INC_TUK_UTILS_H_ */
