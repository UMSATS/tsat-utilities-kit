/** (c) 2024 UMSATS
 * @file mock_can_message.h
 *
 * Mock version of can_message.h for standalone testing.
 * Removes STM32 HAL dependencies to allow testing on any platform.
 */

#ifndef MOCK_CAN_MESSAGE_H_
#define MOCK_CAN_MESSAGE_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CAN_MAX_BODY_SIZE 7

typedef enum
{
	NODE_CDH     = 0,
	NODE_POWER   = 1,
	NODE_ADCS    = 2,
	NODE_PAYLOAD = 3
} NodeID;

#define NODE_ID_MAX 3

// Mock CmdID - in real code this comes from can_command_list.h
typedef uint8_t CmdID;

typedef struct
{
	CmdID cmd;
	uint8_t body[CAN_MAX_BODY_SIZE];
	uint8_t body_size;
	uint8_t priority;
	NodeID sender;
	NodeID recipient;
	uint8_t is_ack;
} CANMessage;

// Mock macros for getting/setting message data
#define GET_MSG_DATA(body, byte_pos, type) ({ \
	type var; \
	memcpy(&var, &body[byte_pos], sizeof(var)); \
	var; \
	})

#define SET_MSG_DATA(body, byte_pos, type, value) do { \
	type var = value; \
	memcpy(&body[byte_pos], &var, sizeof(var)); \
	} while (0)

#endif /* MOCK_CAN_MESSAGE_H_ */
