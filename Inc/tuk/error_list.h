/** (c) 2024 UMSATS
 * @file error_list.h
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_LIST_H_
#define TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_LIST_H_

typedef enum {
	// GENERAL
	ERR_OK = 0,                // No Error.
	ERR_NULL_ARG,              // A function argument is unexpectedly NULL.
	ERR_ARG_OUT_OF_RANGE,      // A function argument is in an unexpected range.
	ERR_INVALID_ARG,           // A function argument is not rational.
	ERR_QUEUE_FULL,            // The operation failed due to a full queue.
	ERR_QUEUE_EMPTY,           // The operation failed due to an empty queue.
	ERR_UNKNOWN_COMMAND,       // Unknown command ID in CAN message.
	ERR_CAN_WRAPPER,           // CAN Wrapper Module error.

	// HAL FUNCTIONS
	ERR_ADC_CALIBRATION_START,
	ERR_ADC_GET_VALUE,
	ERR_ADC_POLL,
	ERR_ADC_START,
	ERR_ADC_STOP,
	ERR_CAN_ACTIVATE_NOTIFICATION,
	ERR_CAN_CONFIG_FILTER,
	ERR_CAN_START,
	ERR_FLASH_LOCK,
	ERR_FLASH_READ_DATA,
	ERR_FLASH_UNLOCK,
	ERR_FLASH_WRITE_DATA,
	ERR_I2C_RECEIVE,            // I2C_Receive returned an error code.
	ERR_I2C_TRANSMIT,           // I2C_Transmit returned an error code.

	// CDH

	// POWER

	// ADCS

	// PAYLOAD
	ERR_PLD_INVALID_WELL_ID,
	ERR_PLD_TCA9539_CLEAR_PINS,
	ERR_PLD_TCA9539_GET_PIN,
	ERR_PLD_TCA9539_GET_PORT,
	ERR_PLD_TCA9539_INIT,
	ERR_PLD_TCA9539_INVALID_EXPANDER_ID,
	ERR_PLD_TCA9539_INVALID_EXPANDER_PIN_ID,
	ERR_PLD_TCA9539_SET_PIN,
	ERR_PLD_TCA9539_SET_PORT,
	ERR_PLD_TCA9548_INIT,
	ERR_PLD_TCA9548_INVALID_CHANNEL,
	ERR_PLD_TCA9548_SET_CHANNEL,
} ErrorCode;

#endif /* TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_LIST_H_ */
