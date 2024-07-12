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
	SENSOR_PCB_TEMP = 0,
	SENSOR_MCU_TEMP,
	SENSOR_PLD_WELL_TEMP,
	SENSOR_PLD_WELL_LIGHT,
} SensorID;

typedef enum
{
	POWER_LINE_BATTERY = 0,
	POWER_LINE_PAYLOAD,
	POWER_LINE_ADCS
} PowerLineID;

typedef enum
{
	////////////////////////////////////////////
	/// COMMON
	////////////////////////////////////////////
	CMD_PREPRARE_FOR_SHUTDOWN            = 0x00,
	CMD_RESET                            = 0x01,
	CMD_GET_PCB_TEMP                     = 0x02,
	CMD_GET_MCU_TEMP                     = 0x03,

	////////////////////////////////////////////
	/// CDH
	////////////////////////////////////////////
	// Event Processing.
	CMD_CDH_PROCESS_HEARTBEAT            = 0x10,
	CMD_CDH_PROCESS_ERROR                = 0x11,
	CMD_CDH_PROCESS_READY_FOR_SHUTDOWN   = 0x12,
	CMD_CDH_PROCESS_STARTUP              = 0x13,
	CMD_CDH_PROCESS_PCB_TEMP             = 0x14,
	CMD_CDH_PROCESS_MCU_TEMP             = 0x15,
	CMD_CDH_PROCESS_CONVERTER_STATUS     = 0x16,
	CMD_CDH_PROCESS_BATTERY_VOLTAGE      = 0x17,
	CMD_CDH_PROCESS_MAGNETIC_FIELD       = 0x18,
	CMD_CDH_PROCESS_ANGULAR_VELOCITY     = 0x19,
	CMD_CDH_PROCESS_WELL_LIGHT           = 0x1A,
	CMD_CDH_PROCESS_WELL_TEMP            = 0x1B,
	CMD_CDH_PROCESS_LED_TEST             = 0x1C,

	// Tests
	CMD_CDH_TEST_FLASH                   = 0x1D,
	CMD_CDH_TEST_MRAM                    = 0x1E,

	// Antenna
	CMD_CDH_ENABLE_ANTENNA_DEPLOYMENT    = 0x1F,
	CMD_CDH_DEPLOY_ANTENNA               = 0x20,
	CMD_CDH_TRANSMIT_UHF_BEACON          = 0x21,

	// RTOS
	CMD_CDH_GET_NUM_TASKS                = 0x22,
	CMD_CDH_SCHEDULE_SAMPLE_TASK         = 0x23,

	// CLock
	CMD_CDH_SET_RTC                      = 0x24,
	CMD_CDH_GET_RTC                      = 0x25,

	CMD_CDH_SET_TELEMETRY_INTERVAL       = 0x26,

	////////////////////////////////////////////
	/// POWER
	////////////////////////////////////////////
	CMD_PWR_SET_LINE_POWER               = 0x30,
	CMD_PWR_SET_BATTERY_HEATER           = 0x31,
	CMD_PWR_GET_CONVERTER_STATUS         = 0x32,
	CMD_PWR_SET_TELEMETRY_INTERVAL       = 0x33,

	////////////////////////////////////////////
	/// ADCS
	////////////////////////////////////////////
	CMD_ADCS_SET_MAGNETORQUER_POWER      = 0x40,
	CMD_ADCS_SET_MAGNETORQUER_DIRECTION  = 0x41,
	CMD_ADCS_GET_MAGNETIC_FIELD          = 0x42,
	CMD_ADCS_GET_ANGULAR_VELOCITY        = 0x43,
	CMD_ADCS_SET_TELEMETRY_INTERVAL      = 0x44,

	////////////////////////////////////////////
	/// PAYLOAD
	////////////////////////////////////////////
	CMD_PLD_SET_WELL_LED                 = 0x50,
	CMD_PLD_SET_WELL_HEATER              = 0x51,
	CMD_PLD_SET_WELL_TEMP                = 0x52,
	CMD_PLD_GET_WELL_TEMP                = 0x53,
	CMD_PLD_GET_WELL_LIGHT               = 0x54,
	CMD_PLD_SET_TELEMETRY_INTERVAL       = 0x55,
	CMD_PLD_TEST_LEDS                    = 0x56,

	////////////////////////////////////////////
	/// GROUND STATION
	////////////////////////////////////////////
	CMD_GND_VERIFY_FLASH_TEST            = 0x60,
	CMD_GND_VERIFY_MRAM_TEST             = 0x61,
	CMD_GDN_VERIFY_CDH_NUM_TASKS         = 0x62,
	CMD_GND_VERIFY_SAMPLE_TASK           = 0x63,
	CMD_GND_VERIFY_RTC                   = 0x64,
} CmdID;

typedef struct
{
	uint8_t body_size;
	uint8_t priority;
} CmdConfig;

extern const CmdConfig cmd_configs[0x70];

#endif /* CAN_WRAPPER_MODULE_INC_CAN_COMMAND_LIST_H_ */
