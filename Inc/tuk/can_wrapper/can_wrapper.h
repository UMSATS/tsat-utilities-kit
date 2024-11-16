/** (c) 2024 UMSATS
 * @file can_wrapper.h
 *
 * CAN wrapper for simplified initialisation, message reception, and message
 * transmission.
 */

#ifndef CAN_WRAPPER_MODULE_INC_CAN_WRAPPER_H_
#define CAN_WRAPPER_MODULE_INC_CAN_WRAPPER_H_

#include "can_command_list.h"
#include "can_message.h"
#include "error_info.h"
#include "error_queue.h"
#include "telemetry_id.h"
#include "cwm_mode.h"
#include "can_wrapper_status.h"

#ifdef CWM_MODE_RTOS
#include "msg_task.h"
#include "cmsis_os.h"
#endif

#include <stdbool.h>
#include <stm32l4xx_hal_can.h>
#include <stm32l4xx_hal_tim.h>
#include <stdint.h>
#include <stddef.h>

typedef void (*CANMessageCallback)(CANMessage);
typedef void (*CANErrorCallback)(CANWrapper_ErrorInfo);

typedef struct
{
	NodeID node_id;           // your subsystem's unique ID in the CAN network.

	CAN_HandleTypeDef *hcan;  // pointer to the CAN peripheral handle.
	TIM_HandleTypeDef *htim;  // pointer to the timer handle.

	CANMessage *msg_queue_buffer; // A block of memory CAN Wrapper will use to store incoming messages.
	size_t msg_queue_buffer_size; // Must be multiple of sizeof(CANMessage).

#ifdef CWM_MODE_RTOS
	CANWrapper_Msg_Task_InitTypeDef msg_task_init_struct; // Settings for the RTOS task.
#endif

	CANMessageCallback message_callback; // called when a new message is polled.
	CANErrorCallback error_callback;     // called when an error occurs.
} CANWrapper_InitTypeDef;

/**
 * @brief              Performs necessary setup for normal functioning.
 *
 * @param init_struct  Configuration for initialisation.
 */
CANWrapper_StatusTypeDef CANWrapper_Init(const CANWrapper_InitTypeDef *init_struct);

/**
 * @brief              Sets the user's node ID (for advanced usage only).
 *
 * @param id           The new ID to be set.
 */
CANWrapper_StatusTypeDef CANWrapper_Set_Node_ID(NodeID id);

/**
 * @brief              Polls for new messages and errors.
 *
 * This is the point where message_callback and error_callback will be called.
 * Does not poll messages in immediate mode.
 */
CANWrapper_StatusTypeDef CANWrapper_Poll_Events();

/**
 * @brief              Sends a message over CAN.
 *
 * @param recipient    ID of the intended recipient.
 * @param msg          See CANMessage definition.
 */
CANWrapper_StatusTypeDef CANWrapper_Transmit(NodeID recipient, const CANMessage *msg);

#endif /* CAN_WRAPPER_MODULE_INC_CAN_WRAPPER_H_ */
