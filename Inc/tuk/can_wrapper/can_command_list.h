/**
 * @file can_command_list.h
 * Configurations for all valid command ID's.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date March 3, 2024
 */

#ifndef CAN_WRAPPER_MODULE_INC_CAN_COMMAND_LIST_H_
#define CAN_WRAPPER_MODULE_INC_CAN_COMMAND_LIST_H_

#include <stdint.h>

typedef enum
{
	////////////////////////////////////////////
	/// COMMON
	////////////////////////////////////////////
	CMD_COMM_RESET                       = 0x00,
	CMD_COMM_PREPARE_FOR_SHUTDOWN        = 0x01,
	CMD_COMM_GET_TELEMETRY               = 0x02,
	CMD_COMM_SET_TELEMETRY_INTERVAL      = 0x03,
	CMD_COMM_GET_TELEMETRY_INTERVAL      = 0x04,
	CMD_COMM_UPDATE_START                = 0x05,
	CMD_COMM_UPDATE_LOAD                 = 0x06,
	CMD_COMM_UPDATE_END                  = 0x07,

	////////////////////////////////////////////
	/// CDH
	////////////////////////////////////////////
	// Event Processing.
	CMD_CDH_PROCESS_HEARTBEAT            = 0x10,
	CMD_CDH_PROCESS_RUNTIME_ERROR        = 0x11,
	CMD_CDH_PROCESS_COMMAND_ERROR        = 0x12,
	CMD_CDH_PROCESS_NOTIFICATION         = 0x13,
	CMD_CDH_PROCESS_TELEMETRY_REPORT     = 0x14,
	CMD_CDH_PROCESS_RETURN               = 0x15,
	CMD_CDH_PROCESS_LED_TEST             = 0x16,

	// Clock
	CMD_CDH_SET_RTC                      = 0x17,
	CMD_CDH_GET_RTC                      = 0x18,

	// Tests
	CMD_CDH_TEST_FLASH                   = 0x19,
	CMD_CDH_TEST_MRAM                    = 0x1A,

	CMD_CDH_RESET_SUBSYSTEM              = 0x1B,

	// Antenna
	CMD_CDH_ENABLE_ANTENNA               = 0x1C,
	CMD_CDH_DEPLOY_ANTENNA               = 0x1D,

	////////////////////////////////////////////
	/// POWER
	////////////////////////////////////////////
	CMD_PWR_PROCESS_HEARTBEAT            = 0x20,
	CMD_PWR_SET_SUBSYSTEM_POWER          = 0x21,
	CMD_PWR_GET_SUBSYSTEM_POWER          = 0x22,
	CMD_PWR_SET_BATTERY_HEATER_POWER     = 0x23,
	CMD_PWR_GET_BATTERY_HEATER_POWER     = 0x24,
	CMD_PWR_SET_BATTERY_ACCESS           = 0x25,
	CMD_PWR_GET_BATTERY_ACCESS           = 0x26,

	////////////////////////////////////////////
	/// ADCS
	////////////////////////////////////////////
	CMD_ADCS_SET_MAGNETORQUER_DIRECTION  = 0x30,
	CMD_ADCS_GET_MAGNETORQUER_DIRECTION  = 0x31,
	CMD_ADCS_SET_OPERATING_MODE          = 0x32,
	CMD_ADCS_GET_OPERATING_MODE          = 0x33,

	////////////////////////////////////////////
	/// PAYLOAD
	////////////////////////////////////////////
	CMD_PLD_SET_ACTIVE_ENVS              = 0x40,
	CMD_PLD_GET_ACTIVE_ENVS              = 0x41,
	CMD_PLD_SET_SETPOINT                 = 0x42,
	CMD_PLD_GET_SETPOINT                 = 0x43,
	CMD_PLD_SET_TOLERANCE                = 0x44,
	CMD_PLD_GET_TOLERANCE                = 0x45,
	CMD_PLD_TEST_LEDS                    = 0x46,

	////////////////////////////////////////////
	/// GROUND STATION
	////////////////////////////////////////////
	CMD_GND_VERIFY_FLASH_TEST            = 0x50,
	CMD_GND_VERIFY_MRAM_TEST             = 0x51,
	CMD_GDN_VERIFY_CDH_NUM_TASKS         = 0x52,
	CMD_GND_VERIFY_SAMPLE_TASK           = 0x53,
	CMD_GND_VERIFY_RTC                   = 0x54,
} CmdID;

typedef struct
{
	uint8_t body_size;
	uint8_t priority;
} CmdConfig;

extern const CmdConfig cmd_configs[0x70];

#endif /* CAN_WRAPPER_MODULE_INC_CAN_COMMAND_LIST_H_ */
