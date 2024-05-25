/**
 * @file can_command_list.c
 * Configurations for all valid command ID's.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date March 16, 2024
 */

#include "can_command_list.h"

const CmdConfig cmd_configs[0x70] = {
		//////////////////////////////////////////////////////////////
		/// COMMON
		//////////////////////////////////////////////////////////////
		///CMD                                 //BODY SIZE //PRIORITY
		[CMD_PREPRARE_FOR_SHUTDOWN]            ={0,          0       },
		[CMD_RESET]                            ={0,          0       },
		[CMD_GET_PCB_TEMP]                     ={0,          0       },
		[CMD_GET_MCU_TEMP]                     ={0,          0       },

		//////////////////////////////////////////////////////////////
		/// CDH
		//////////////////////////////////////////////////////////////
		///CMD                                 //BODY SIZE //PRIORITY
		[CMD_CDH_PROCESS_HEARTBEAT]            ={0,          0       },
		[CMD_CDH_PROCESS_ERROR]                ={7,          0       },
		[CMD_CDH_PROCESS_READY_FOR_SHUTDOWN]   ={0,          0       },
		[CMD_CDH_PROCESS_STARTUP]              ={0,          0       },
		[CMD_CDH_PROCESS_PCB_TEMP]             ={2,          0       },
		[CMD_CDH_PROCESS_MCU_TEMP]             ={2,          0       },
		[CMD_CDH_PROCESS_CONVERTER_STATUS]     ={1,          0       },
		[CMD_CDH_PROCESS_BATTERY_VOLTAGE]      ={0,          0       },
		[CMD_CDH_PROCESS_WELL_LIGHT]           ={3,          0       },
		[CMD_CDH_PROCESS_WELL_TEMP]            ={3,          0       },
		[CMD_CDH_PROCESS_LED_TEST]             ={2,          0       },

		[CMD_CDH_TEST_FLASH]                   ={0,          0       },
		[CMD_CHD_TEST_MRAM]                    ={0,          0       },

		[CMD_CDH_ENABLE_ANTENNA_DEPLOYMENT]    ={0,          0       },
		[CMD_CDH_DEPLOY_ANTENNA]               ={0,          0       },
		[CMD_CDH_TRANSMIT_UHF_BEACON]          ={0,          0       },

		[CMD_CDH_GET_NUM_TASKS]                ={0,          0       },
		[CMD_CDH_SCHEDULE_SAMPLE_TASK]         ={4,          0       },

		[CMD_CDH_SET_RTC]                      ={4,          0       },
		[CMD_CDH_GET_RTC]                      ={0,          0       },

		[CMD_CDH_SET_TELEMETRY_INTERVAL]       ={3,          0       },

		//////////////////////////////////////////////////////////////
		/// POWER
		//////////////////////////////////////////////////////////////
		///CMD                                 //BODY SIZE //PRIORITY
		[CMD_PWR_SET_LINE_POWER]               ={2,          0       },
		[CMD_PWR_SET_BATTERY_HEATER]           ={1,          0       },
		[CMD_PWR_GET_CONVERTER_STATUS]         ={0,          0       },
		[CMD_PWR_SET_TELEMETRY_INTERVAL]       ={3,          0       },

		//////////////////////////////////////////////////////////////
		/// ADCS
		//////////////////////////////////////////////////////////////
		///CMD                                 //BODY SIZE //PRIORITY
		[CMD_ADCS_SET_MAGNETORQUER_POWER]      ={2,          0       },
		[CMD_ADCS_SET_MAGNETORQUER_DIRECTION]  ={2,          0       },
		[CMD_ADCS_SET_TELEMETRY_INTERVAL]      ={3,          0       },

		//////////////////////////////////////////////////////////////
		/// PAYLOAD
		//////////////////////////////////////////////////////////////
		///CMD                                 //BODY SIZE //PRIORITY
		[CMD_PLD_SET_WELL_LED]                 ={2,          0       },
		[CMD_PLD_SET_WELL_HEATER]              ={2,          0       },
		[CMD_PLD_SET_WELL_TEMP]                ={3,          0       },
		[CMD_PLD_GET_WELL_TEMP]                ={1,          0       },
		[CMD_PLD_GET_WELL_LIGHT]               ={1,          0       },
		[CMD_PLD_SET_TELEMETRY_INTERVAL]       ={4,          0       },
		[CMD_PLD_TEST_LEDS]                    ={0,          0       },
};
