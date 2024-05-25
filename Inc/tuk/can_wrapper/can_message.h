/**
 * @file can_message.h
 * Structures for storing CAN message data.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
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

typedef union
{
	struct
	{
		uint8_t cmd;
		uint8_t body[CAN_MAX_BODY_SIZE];
	};
	uint8_t data[CAN_MAX_BODY_SIZE+1];
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

// macros to get/set arguments in a command.
// NOTE: these depend on the fact that TSAT's MCUs are all of the same endianness.
// If that were to change, you would have to set/get the elements in the message
// body directly.

#define GET_ARG(msg, pos, var) \
	var = *((typeof(var)*)(msg.body + pos))

#define SET_ARG(msg, pos, var) \
	*((typeof(var)*)(msg.body + pos)) = var;

#endif /* CAN_WRAPPER_MODULE_INC_CAN_MESSAGE_H_ */
