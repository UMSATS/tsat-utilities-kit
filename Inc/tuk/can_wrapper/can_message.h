/** (c) 2024 UMSATS
 * @file can_message.h
 *
 * Structures for storing CAN message data.
 */

#ifndef CAN_WRAPPER_MODULE_INC_CAN_MESSAGE_H_
#define CAN_WRAPPER_MODULE_INC_CAN_MESSAGE_H_

#include "can_command_list.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define CAN_MAX_BODY_SIZE 7

typedef enum
{
	NODE_CDH     = 0,
	NODE_POWER   = 1,
	NODE_ADCS    = 2,
	NODE_PAYLOAD = 3
} NodeID;
_Static_assert(sizeof(NodeID) == 1, "Enum size exceeds 1 byte");

#define NODE_ID_MAX 3

typedef struct
{
	CmdID cmd;
	uint8_t body[CAN_MAX_BODY_SIZE];
	uint8_t priority;
	NodeID sender;
	NodeID recipient;
	uint8_t is_ack;
} CANMessage;

// Macros to get & set arguments in a command.
// NOTE: These assume consistent endianness across subsystems. If a subsystem
// switches to a big-endian processor, that subsystem will have to perform a
// byte swap of the message contents.

// Example Usage: float arg = GET_MSG_DATA(msg, 0, float);
#define GET_MSG_DATA(msg, pos, type) ({ \
	type var; \
	memcpy(&var, &msg.body[pos], sizeof(var)); \
	var; \
	})

// Example Usage: SET_MSG_DATA(msg, 0, uint8_t, 32);
#define SET_MSG_DATA(msg, pos, type, value) do { \
	type var = value; \
	memcpy(&msg.body[pos], &var, sizeof(var)); \
	} while (0)

#endif /* CAN_WRAPPER_MODULE_INC_CAN_MESSAGE_H_ */
