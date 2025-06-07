/** (c) 2024 UMSATS
 * @file can_wrapper.h
 *
 * CAN wrapper for simplified initialisation, message reception, and message
 * transmission.
 */

#ifndef CAN_WRAPPER_MODULE_INC_CAN_WRAPPER_H_
#define CAN_WRAPPER_MODULE_INC_CAN_WRAPPER_H_

#include "hal_include.h"

#include "can_command_list.h"
#include "can_message.h"
#include "can_queue.h"
#include "error_info.h"
#include "error_queue.h"
#include "telemetry_id.h"

#include "tuk/error_list.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef void (*CANMessageCallback)(CAN_HandleTypeDef*, CANMessage);
typedef void (*CANCommandHandlerCallback)(CANMessage);
typedef void (*CANErrorCallback)(CANWrapper_ErrorInfo);

typedef struct
{
	NodeID node_id;           // your subsystem's unique ID in the CAN network.
	TIM_HandleTypeDef *htim;  // pointer to the timer handle.
	CANMessageCallback message_callback; // called when a new message is received.
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
 * @brief              Initializes and starts a CAN peripheral.
 * @warning            This function should not be called from an ISR.
 *
 * @param hcan         Handle of the peripheral.
 */
ErrorCode CANWrapper_CAN_Start(CAN_HandleTypeDef *hcan);

/**
 * @brief              Sets the user's node ID (for advanced usage only).
 *
 * @param id           The new ID to be set.
 */
ErrorCode CANWrapper_Set_Node_ID(NodeID id);

/**
 * @brief              Polls a CAN queue for messages, processing ACK's and
 *                     invoking the callback for each non-ACK message.
 * @warning            This function should not be called from an ISR.
 *
 * @param can_queue    The queue to poll messages from.
 * @param callback     Callback for your command handler function.
 * @return
 */
ErrorCode CANWrapper_Poll_CAN_Queue(CANQueue *can_queue, CANCommandHandlerCallback callback);

/**
 * @brief              Polls errors from the CAN controller.
 * @warning            This function should not be called from an ISR.
 *
 * This is the point where error_callback will be called.
 */
ErrorCode CANWrapper_Poll_Errors();

/**
 * @brief              Sends a message over CAN.
 * @note               This function may fail if called from an ISR. Therefore,
 *                     avoid doing so unless you have considered recovery in the
 *                     case of an error.
 *
 * @param hcan         Handle of the CAN peripheral.
 * @param recipient    ID of the intended recipient.
 * @param cmd_id       Command being sent. This determines what goes in `body`.
 * @param body         The bytes to transmit.
 */
ErrorCode CANWrapper_Transmit(CAN_HandleTypeDef *hcan, NodeID recipient, CmdID cmd_id, const uint8_t *body);

/**
 * @brief              Sends an acknowledgement over CAN.
 * @warning            This function should not be called from an ISR.
 *
 * @param hcan         Handle of the CAN peripheral.
 * @param msg          The message to acknowledge.
 */
ErrorCode CANWrapper_Transmit_Ack(CAN_HandleTypeDef *hcan, const CANMessage *msg);

/**
 * @brief              Removes an acknowledged message from the timeout detection queue.
 * @warning            This function should not be called from an ISR.
 *
 * @param msg          The ack message.
 */
ErrorCode CANWrapper_Process_Ack(const CANMessage *msg);

#endif /* CAN_WRAPPER_MODULE_INC_CAN_WRAPPER_H_ */
