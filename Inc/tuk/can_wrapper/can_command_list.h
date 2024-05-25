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
	CMD_RESET,
	CMD_GET_PCB_TEMP,
	CMD_GET_MCU_TEMP,

	////////////////////////////////////////////
	/// CDH
	////////////////////////////////////////////
	// Event Processing.
	CMD_CDH_PROCESS_HEARTBEAT            = 0x10,
	CMD_CDH_PROCESS_ERROR,
	CMD_CDH_PROCESS_READY_FOR_SHUTDOWN,
	CMD_CDH_PROCESS_STARTUP,
	CMD_CDH_PROCESS_PCB_TEMP,
	CMD_CDH_PROCESS_MCU_TEMP,
	CMD_CDH_PROCESS_CONVERTER_STATUS,
	CMD_CDH_PROCESS_BATTERY_VOLTAGE,
	CMD_CDH_PROCESS_WELL_LIGHT,
	CMD_CDH_PROCESS_WELL_TEMP,
	CMD_CDH_PROCESS_LED_TEST,

	// Tests
	CMD_CDH_TEST_FLASH,
	CMD_CHD_TEST_MRAM,

	// Antenna
	CMD_CDH_ENABLE_ANTENNA_DEPLOYMENT,
	CMD_CDH_DEPLOY_ANTENNA,
	CMD_CDH_TRANSMIT_UHF_BEACON,

	// RTOS
	CMD_CDH_GET_NUM_TASKS,
	CMD_CDH_SCHEDULE_SAMPLE_TASK,

	// CLock
	CMD_CDH_SET_RTC,
	CMD_CDH_GET_RTC,

	CMD_CDH_SET_TELEMETRY_INTERVAL,

	////////////////////////////////////////////
	/// POWER
	////////////////////////////////////////////
	CMD_PWR_SET_LINE_POWER               = 0x30,
	CMD_PWR_SET_BATTERY_HEATER,
	CMD_PWR_GET_CONVERTER_STATUS,
	CMD_PWR_SET_TELEMETRY_INTERVAL,

	////////////////////////////////////////////
	/// ADCS
	////////////////////////////////////////////
	CMD_ADCS_SET_MAGNETORQUER_POWER       = 0x40,
	CMD_ADCS_SET_MAGNETORQUER_DIRECTION,
	CMD_ADCS_SET_TELEMETRY_INTERVAL,

	////////////////////////////////////////////
	/// PAYLOAD
	////////////////////////////////////////////
	CMD_PLD_SET_WELL_LED                 = 0x50,
	CMD_PLD_SET_WELL_HEATER,
	CMD_PLD_SET_WELL_TEMP,
	CMD_PLD_GET_WELL_TEMP,
	CMD_PLD_GET_WELL_LIGHT,
	CMD_PLD_SET_TELEMETRY_INTERVAL,
	CMD_PLD_TEST_LEDS,

	////////////////////////////////////////////
	/// GROUND STATION
	////////////////////////////////////////////
	CMD_GND_VERIFY_FLASH_TEST             = 0x60,
	CMD_GND_VERIFY_MRAM_TEST,
	CMD_GDN_VERIFY_CDH_NUM_TASKS,
	CMD_GND_VERIFY_SAMPLE_TASK,
	CMD_GND_VERIFY_RTC
} CmdID;

typedef struct
{
	uint8_t body_size;
	uint8_t priority;
} CmdConfig;

extern const CmdConfig cmd_configs[0x70];

#endif /* CAN_WRAPPER_MODULE_INC_CAN_COMMAND_LIST_H_ */
