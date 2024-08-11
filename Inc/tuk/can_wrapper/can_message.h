/**
 * @file can_message.h
 * Structures for storing CAN message data.
 *
 * @date March 6, 2024
 */

#ifndef CAN_WRAPPER_MODULE_INC_CAN_MESSAGE_H_
#define CAN_WRAPPER_MODULE_INC_CAN_MESSAGE_H_

#include "can_command_list.h"

#include <sys/_stdint.h>
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

typedef struct
{
	uint8_t cmd;
	uint8_t body[CAN_MAX_BODY_SIZE];
} CANMessage;

typedef struct
{
	CANMessage msg;
	uint8_t priority;
	uint8_t sender;
	uint8_t recipient;
	uint8_t is_ack;
} CachedCANMessage;

static inline bool CANMessage_Equals(const CANMessage *msg1, const CANMessage *msg2)
{
	return msg1->cmd == msg2->cmd
			&& memcmp(msg1->body, msg2->body, cmd_configs[msg1->cmd].body_size) == 0;
}

// Macros to get & set arguments in a command.
// NOTE: These assume consistent endianness across subsystems. If a subsystem
// switches to a big-endian processor, that subsystem will have to perform a
// byte swap.

// Example Usage: float arg = GET_ARG(msg, 0, float);
#define GET_ARG(msg, pos, type) ({ \
	type var; \
	memcpy(&var, &msg.body[pos], sizeof(var)); \
	var; \
	})

// Example Usage: SET_ARG(msg, 0, uint8_t, 32);
#define SET_ARG(msg, pos, type, value) do { \
	type var = value; \
	memcpy(&msg.body[pos], &var, sizeof(var)); \
	} while (0)

#endif /* CAN_WRAPPER_MODULE_INC_CAN_MESSAGE_H_ */
