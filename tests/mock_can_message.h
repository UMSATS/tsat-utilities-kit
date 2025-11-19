/** (c) 2024 UMSATS
 * @file mock_can_message.h
 * @brief Mock CAN message types for standalone testing (no STM32 HAL dependencies)
 */

#ifndef MOCK_CAN_MESSAGE_H_
#define MOCK_CAN_MESSAGE_H_

#include <stdint.h>
#include <stdbool.h>

#define CAN_MAX_BODY_SIZE 8

typedef enum {
	NODE_CDH = 0,
	NODE_POWER = 1,
	NODE_ADCS = 2,
	NODE_PAYLOAD = 3
} NodeID;

typedef uint8_t CmdID;

typedef struct {
	CmdID cmd;
	uint8_t body[CAN_MAX_BODY_SIZE];
	uint8_t body_size;
	uint8_t priority;
	NodeID sender;
	NodeID recipient;
	uint8_t is_ack;
} CANMessage;

#endif /* MOCK_CAN_MESSAGE_H_ */
