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

#ifdef CWM_MODE_RTOS
#include "msg_task.h"
#include "cmsis_os.h"
#endif

#include "tuk/error_list.h"

#include <stdbool.h>
#include <stm32l4xx.h>
#include <stm32l4xx_hal_def.h>
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

#ifdef CWM_MODE_RTOS
	osMessageQueueId_t msg_queue; // Settings for the RTOS queue.
#else
	CANMessage *msg_queue_buffer; // A block of memory CAN Wrapper will use to store incoming messages.
	size_t msg_queue_buffer_size; // Must be multiple of sizeof(CANMessage).
#endif

	CANMessageCallback message_callback; // called when a new message is polled.
	CANErrorCallback error_callback;     // called when an error occurs.
} CANWrapper_InitTypeDef;

/**
 * @brief              Performs necessary setup for normal functioning.
 * @warning            This function should not be called from an ISR.
 *
 * @param init_struct  Configuration for initialisation.
 */
ErrorCode CANWrapper_Init(const CANWrapper_InitTypeDef *init_struct);

/**
 * @brief              Sets the user's node ID (for advanced usage only).
 *
 * @param id           The new ID to be set.
 */
ErrorCode CANWrapper_Set_Node_ID(NodeID id);

#ifndef CWM_MODE_RTOS
/**
 * @brief              Polls for new messages and errors.
 * @warning            This function should not be called from an ISR.
 * @note               Only available when compiling in Manual or Normal mode.
 *
 * This is the point where message_callback and error_callback will be called.
 */
ErrorCode CANWrapper_Poll_Events();
#else
// Switch to Poll_Errors function for RTOS applications. This forces the user to
// think about what they are polling, which is important when working with RTOS.
/**
 * @brief              Polls for new errors.
 * @warning            This function should not be called from an ISR.
 * @note               Only available when compiling in RTOS mode.
 *
 * This is the point where error_callback will be called.
 */
ErrorCode CANWrapper_Poll_Errors();
#endif

/**
 * @brief              Sends a message over CAN.
 * @warning            This function should not be called from an ISR.
 *
 * @param recipient    ID of the intended recipient.
 * @param msg          See CANMessage definition.
 */
ErrorCode CANWrapper_Transmit(NodeID recipient, const CANMessage *msg);

#endif /* CAN_WRAPPER_MODULE_INC_CAN_WRAPPER_H_ */
