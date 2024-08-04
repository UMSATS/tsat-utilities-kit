/**
 * @file can_command_list.c
 * Configurations for all valid command ID's.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date March 16, 2024
 */

#include <tuk/can_wrapper/can_command_list.h>

const CmdConfig cmd_configs[0x70] = {
		////////////////////////////////////////////
		/// COMMON
		////////////////////////////////////////////
		//CMD                                 //BODY SIZE //PRIORITY
		[CMD_COMM_RESET]                       ={0,   0b000000       },
		[CMD_COMM_PREPARE_FOR_SHUTDOWN]        ={0,   0b000001       },
		[CMD_COMM_GET_TELEMETRY]               ={1,   0b100000       },
		[CMD_COMM_SET_TELEMETRY_INTERVAL]      ={3,   0b100000       },
		[CMD_COMM_GET_TELEMETRY_INTERVAL]      ={1,   0b100000       },
		[CMD_COMM_UPDATE_START]                ={4,   0b100000       },
		[CMD_COMM_UPDATE_LOAD]                 ={7,   0b100000       },
		[CMD_COMM_UPDATE_END]                  ={0,   0b100000       },

		////////////////////////////////////////////
		/// CDH
		////////////////////////////////////////////
		//CMD                                 //BODY SIZE //PRIORITY
		[CMD_CDH_PROCESS_HEARTBEAT]            ={0,   0b000010       },
		[CMD_CDH_PROCESS_RUNTIME_ERROR]        ={2,   0b001010       },
		[CMD_CDH_PROCESS_COMMAND_ERROR]        ={2,   0b001010       },
		[CMD_CDH_PROCESS_NOTIFICATION]         ={1,   0b000010       },
		[CMD_CDH_PROCESS_TELEMETRY_REPORT]     ={7,   0b000011       },
		[CMD_CDH_PROCESS_RETURN]               ={1,   0b100000       }, // Remaining bytes correspond to cmd ID
		[CMD_CDH_PROCESS_LED_TEST]             ={2,   0b000100       },

		[CMD_CDH_SET_RTC]                      ={4,   0b100000       },
		[CMD_CDH_GET_RTC]                      ={0,   0b100000       },

		[CMD_CDH_TEST_FLASH]                   ={0,   0b100000       },
		[CMD_CDH_TEST_MRAM]                    ={0,   0b100000       },

		[CMD_CDH_RESET_SUBSYSTEM]              ={4,   0b100000       },

		[CMD_CDH_ENABLE_ANTENNA]               ={0,   0b100000       },
		[CMD_CDH_DEPLOY_ANTENNA]               ={0,   0b100000       },

		////////////////////////////////////////////
		/// POWER
		////////////////////////////////////////////
		//CMD                                 //BODY SIZE //PRIORITY
		[CMD_PWR_PROCESS_HEARTBEAT]            ={0,   0b000010       },
		[CMD_PWR_SET_SUBSYSTEM_POWER]          ={5,   0b000000       },
		[CMD_PWR_GET_SUBSYSTEM_POWER]          ={4,   0b100000       },
		[CMD_PWR_SET_BATTERY_HEATER_POWER]     ={1,   0b000101       },
		[CMD_PWR_GET_BATTERY_HEATER_POWER]     ={0,   0b100000       },
		[CMD_PWR_SET_BATTERY_ACCESS]           ={1,   0b100000       },
		[CMD_PWR_GET_BATTERY_ACCESS]           ={0,   0b100000       },

		////////////////////////////////////////////
		/// ADCS
		////////////////////////////////////////////
		//CMD                                 //BODY SIZE //PRIORITY
		[CMD_ADCS_SET_MAGNETORQUER_DIRECTION]  ={2,   0b100000       },
		[CMD_ADCS_GET_MAGNETORQUER_DIRECTION]  ={1,   0b100000       },
		[CMD_ADCS_SET_OPERATING_MODE]          ={1,   0b100000       },
		[CMD_ADCS_GET_OPERATING_MODE]          ={0,   0b100000       },

		////////////////////////////////////////////
		/// PAYLOAD
		////////////////////////////////////////////
		//CMD                                 //BODY SIZE //PRIORITY
		[CMD_PLD_SET_ACTIVE_ENVS]              ={2,   0b100000       },
		[CMD_PLD_GET_ACTIVE_ENVS]              ={0,   0b100000       },
		[CMD_PLD_SET_SETPOINT]                 ={5,   0b100000       }, // confirm float = 4B
		[CMD_PLD_GET_SETPOINT]                 ={1,   0b100000       },
		[CMD_PLD_SET_TOLERANCE]                ={4,   0b100000       }, // confirm float = 4B
		[CMD_PLD_GET_TOLERANCE]                ={0,   0b100000       },
		[CMD_PLD_TEST_LEDS]                    ={0,   0b000100       },
};
