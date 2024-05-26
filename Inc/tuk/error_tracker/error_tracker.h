/*
 * error_tracker.h
 *
 *  Created on: Jan 15, 2024
 *      Author: Logan Furedi
 */

#ifndef INC_ERROR_CONTEXT_H_
#define INC_ERROR_CONTEXT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
	// COMMON
	ERROR_ADC_CALIBRATION_START = 0,
	ERROR_ADC_GET_VALUE,
	ERROR_ADC_POLL,
	ERROR_ADC_START,
	ERROR_ADC_STOP,
	ERROR_CAN_ACTIVATE_NOTIFICATION,
	ERROR_CAN_CONFIG_FILTER,
	ERROR_CAN_START,
	ERROR_CAN_WRAPPER_INIT,
	ERROR_FLASH_LOCK,
	ERROR_FLASH_READ_DATA,
	ERROR_FLASH_UNLOCK,
	ERROR_FLASH_WRITE_DATA,
	ERROR_I2C_RECEIVE,
	ERROR_I2C_TRANSMIT,
	ERROR_UNKNOWN_COMMAND,

	// PAYLOAD
	ERROR_PLD_INVALID_WELL_ID,
	ERROR_PLD_TCA9539_CLEAR_PINS,
	ERROR_PLD_TCA9539_GET_PIN,
	ERROR_PLD_TCA9539_GET_PORT,
	ERROR_PLD_TCA9539_INIT,
	ERROR_PLD_TCA9539_INVALID_EXPANDER_ID,
	ERROR_PLD_TCA9539_INVALID_EXPANDER_PIN_ID,
	ERROR_PLD_TCA9539_SET_PIN,
	ERROR_PLD_TCA9539_SET_PORT,
	ERROR_PLD_TCA9548_INIT,
	ERROR_PLD_TCA9548_INVALID_CHANNEL,
	ERROR_PLD_TCA9548_SET_CHANNEL,
} ErrorID;

#define ERROR_BUFFER_CAPACITY 6

typedef struct
{
	uint8_t data[ERROR_BUFFER_CAPACITY]; // buffer containing error data.
	size_t size;
} ErrorBuffer;

void ErrorTracker_Init(ErrorBuffer *init_error_buffer);
void ErrorTracker_Push_Buffer(ErrorBuffer *error_buffer);
void ErrorTracker_Pop_Buffer();

bool ErrorBuffer_Has_Error(ErrorBuffer *error_buffer);
void ErrorBuffer_Clear(ErrorBuffer *error_buffer);

// internal function used by PUT_ERROR macro.
void ErrorTracker_Put_Error_(int n, ...);

#define PUT_ERROR(...)ErrorTracker_Put_Error_(NUM_ARGS(__VA_ARGS__),__VA_ARGS__)

#endif /* INC_ERROR_CONTEXT_H_ */
