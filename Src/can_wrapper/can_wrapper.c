/** (c) 2024 UMSATS
 * @file can_wrapper.c
 *
 * CAN wrapper for simplified message receipt & transmission.
 */

#include "tuk/can_wrapper/can_wrapper.h"
#include "tuk/can_wrapper/can_queue.h"
#include "tuk/can_wrapper/error_queue.h"
#include "tuk/can_wrapper/tx_cache.h"
#include "tuk/can_wrapper/cwm_mode.h"
#include "tuk/debug.h"
#include "tuk/utils.h"

#include <stddef.h>

#define ACK_MASK       0b00000000001
#define RECIPIENT_MASK 0b00000000110
#define SENDER_MASK    0b00000011000
#define PRIORITY_MASK  0b11111100000

#define TIMEOUT 3600

#define PERIOD_TICKS 5000

static CANWrapper_InitTypeDef s_init_struct = {0};

// Define message queue.
#ifdef CWM_MODE_RTOS
static osMessageQueueId_t s_msg_queue;
#else
static CANQueue s_msg_queue;
#endif

static TxCache s_tx_cache = {0};
static ErrorQueue s_error_queue = {0};

static bool s_init = false;

static CANWrapper_StatusTypeDef transmit_internal(NodeID recipient, const CANMessage *msg, bool is_ack);
#ifndef CWM_MODE_RTOS
static void CANWrapper_Process_Message(CANMessage *msg);
#endif

CANWrapper_StatusTypeDef CANWrapper_Init(const CANWrapper_InitTypeDef *init_struct)
{
	ASSERT_PARAM(init_struct->node_id <= NODE_ID_MAX, CAN_WRAPPER_ARG_OUT_OF_RANGE);
	ASSERT_PARAM(init_struct->message_callback != NULL, CAN_WRAPPER_NULL_ARG);
	ASSERT_PARAM(init_struct->hcan != NULL, CAN_WRAPPER_NULL_ARG);
	ASSERT_PARAM(init_struct->htim != NULL, CAN_WRAPPER_NULL_ARG);
#ifdef CWM_MODE_RTOS
	ASSERT_PARAM(init_struct->msg_queue != NULL, CAN_WRAPPER_NULL_ARG);
#else
	ASSERT_PARAM(init_struct->msg_queue_buffer != NULL, CAN_WRAPPER_NULL_ARG);
	ASSERT_PARAM(init_struct->msg_queue_buffer_size % sizeof(CANMessage) == 0, CAN_WRAPPER_INVALID_ARG);
#endif

	s_init_struct = *init_struct;

	const CAN_FilterTypeDef filter_config = {
			.FilterIdHigh         = 0x0000,
			.FilterIdLow          = 0x0000,
			.FilterMaskIdHigh     = 0x0000,
			.FilterMaskIdLow      = 0x0000,
			.FilterFIFOAssignment = CAN_FILTER_FIFO0,
			.FilterBank           = 0,
			.FilterMode           = CAN_FILTERMODE_IDMASK,
			.FilterScale          = CAN_FILTERSCALE_32BIT,
			.FilterActivation     = ENABLE,
			.SlaveStartFilterBank = 14,
	};

	if (HAL_CAN_ConfigFilter(s_init_struct.hcan, &filter_config) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_CONFIG_FILTER;
	}

	if (HAL_CAN_Start(s_init_struct.hcan) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_START_CAN;
	}

	// enable CAN interrupt.
	if (HAL_CAN_ActivateNotification(s_init_struct.hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_ENABLE_INTERRUPT;
	}

	if (HAL_TIM_Base_Start(s_init_struct.htim) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_START_TIMER;
	}

#ifdef CWM_MODE_RTOS
	s_msg_queue = s_init_struct.msg_queue;
	CANWrapper_Init_Message_Task(s_init_struct.msg_queue);
#else
	CANQueue_Init(&s_msg_queue, s_init_struct.msg_queue_buffer, s_init_struct.msg_queue_buffer_size);
#endif

	s_tx_cache = TxCache_Create();
	s_error_queue = ErrorQueue_Create();

	s_init = true;
	return CAN_WRAPPER_HAL_OK;
}

CANWrapper_StatusTypeDef CANWrapper_Set_Node_ID(NodeID id)
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;
	if (id > NODE_ID_MAX) return CAN_WRAPPER_INVALID_ARG;

	s_init_struct.node_id = id;

	return CAN_WRAPPER_HAL_OK;
}

CANWrapper_StatusTypeDef CANWrapper_Poll_Events()
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

#ifndef CWM_MODE_RTOS
	/**************************************************
	 *               Message Processing               *
	 **************************************************/

	// Process incoming message queue.
	CANMessage msg;
	while (ERR_OK == CANQueue_Dequeue(&s_msg_queue, &msg))
	{
		CANWrapper_Process_Message(&msg);
	}
#endif
	/**************************************************
	 *                Error Processing                *
	 **************************************************/

	// Get timer values.
	uint32_t counter_value = __HAL_TIM_GET_COUNTER(s_init_struct.htim);
	uint32_t rcr_value = s_init_struct.htim->Instance->RCR;
	uint64_t current_tick = counter_value + rcr_value*PERIOD_TICKS;

	// Poll transmission timeout events.
	while (s_tx_cache.size > 0)
	{
		const TxCacheItem *front_item = &s_tx_cache.items[0];

		// Calculate the times of important events.
		uint64_t transmission_tick = front_item->timestamp.counter_value + front_item->timestamp.rcr_value*PERIOD_TICKS;
		uint64_t timeout_tick = (transmission_tick + TIMEOUT) % (16*PERIOD_TICKS);

		bool clock_overflowed = transmission_tick >= timeout_tick;
		bool timeout_occurred = clock_overflowed ?
		         ( current_tick >= timeout_tick && current_tick < transmission_tick )
		       : ( current_tick >= timeout_tick || current_tick < transmission_tick );

		if (timeout_occurred)
		{
			CANWrapper_ErrorInfo error_info;
			error_info.error = CAN_WRAPPER_ERROR_TIMEOUT;
			error_info.msg = front_item->msg;
			ErrorQueue_Enqueue(&s_error_queue, error_info);

			TxCache_Erase(&s_tx_cache, 0);
		}
		else
		{
			break;
		}
	}

	// Report queued errors
	CANWrapper_ErrorInfo error;
	while (ErrorQueue_Dequeue(&s_error_queue, &error))
	{
		s_init_struct.error_callback(error);
	}

	return CAN_WRAPPER_HAL_OK;
}

CANWrapper_StatusTypeDef CANWrapper_Transmit(NodeID recipient, const CANMessage *msg)
{
	return transmit_internal(recipient, msg, false);
}

static CANWrapper_StatusTypeDef transmit_internal(NodeID recipient, const CANMessage *msg, bool is_ack)
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	if (HAL_CAN_GetState(s_init_struct.hcan) != HAL_CAN_STATE_READY &&
			HAL_CAN_GetState(s_init_struct.hcan) != HAL_CAN_STATE_LISTENING)
	{
		return CAN_WRAPPER_TX_FAIL_BAD_CAN_STATE;
	}

	// Define the message header.
	CAN_TxHeaderTypeDef tx_header = {0};
	tx_header.StdId = (cmd_configs[msg->cmd].priority << 5 & PRIORITY_MASK)
	                | (s_init_struct.node_id          << 3 & SENDER_MASK)
	                | (recipient                      << 1 & RECIPIENT_MASK)
	                | (is_ack                              & ACK_MASK);
	tx_header.IDE = CAN_ID_STD;   // We are using the standard identifier.
	tx_header.RTR = CAN_RTR_DATA; // We are transmitting data.
	tx_header.DLC = 1 + cmd_configs[msg->cmd].body_size; // Length = cmd ID + body.

	// Wait to send CAN message.
	uint16_t limiter = 0;
	while (HAL_CAN_GetTxMailboxesFreeLevel(s_init_struct.hcan) == 0)
	{
		// return if mailboxes aren't being freed.
		// for reasoning of the limit see:
		// https://github.com/UMSATS/tsat-utilities-kit/issues/31#issuecomment-2287744661
		if (limiter >= 4000) return CAN_WRAPPER_TX_MAILBOXES_FULL;
		limiter++;
	}

	uint32_t tx_mailbox; // transmit mailbox.
	HAL_StatusTypeDef status;
	status = HAL_CAN_AddTxMessage(s_init_struct.hcan, &tx_header, (uint8_t*)&msg, &tx_mailbox);

	if (status == HAL_OK && !is_ack)
	{
		// Get timer variables.
		uint32_t counter_value = __HAL_TIM_GET_COUNTER(s_init_struct.htim);
		uint32_t rcr_value = s_init_struct.htim->Instance->RCR;

		TxCacheItem tx_cache_item = {
				.timestamp = {
						.counter_value = counter_value,
						.rcr_value = rcr_value
				},
				.msg = {
						.priority = cmd_configs[msg->cmd].priority,
						.sender = s_init_struct.node_id,
						.recipient = recipient,
						.is_ack = is_ack,
				}
		};

		// Copy over the message contents (command ID + body).
		memcpy(&tx_cache_item.msg.cmd, &msg->cmd, CAN_MAX_BODY_SIZE+1);

		TxCache_Push_Back(&s_tx_cache, &tx_cache_item);
	}

	return status;
}

// called by HAL when a new CAN message is received and pending.
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan == s_init_struct.hcan)
	{
		HAL_StatusTypeDef status;

		// Create an item to be placed into the queue.
		CANMessage msg = {0};

		// Retrieve the message header and body.
		CAN_RxHeaderTypeDef rx_header;
		status = HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, (uint8_t*)&msg.cmd);

		if (status != HAL_OK)
			return; // in theory this should never happen. :p

		// Extract all the metadata from the header.
		msg.priority  = (PRIORITY_MASK & rx_header.StdId) >> 5;
		msg.sender    = (SENDER_MASK & rx_header.StdId) >> 3;
		msg.recipient = (RECIPIENT_MASK & rx_header.StdId) >> 1;
		msg.is_ack    = ACK_MASK & rx_header.StdId;

#ifdef CWM_MODE_MANUAL
		// Enqueue all traffic.
		IGNORE_FAIL(CANQueue_Enqueue(&s_msg_queue, &msg));
#else
		// Check if the message is for us. (TODO: consider CAN filtering instead)
		if (msg.recipient == s_init_struct.node_id && msg.sender != s_init_struct.node_id)
		{
#if defined(CWM_MODE_NORMAL)
			IGNORE_FAIL(CANQueue_Enqueue(&s_msg_queue, &msg));
#elif defined(CWM_MODE_RTOS)
			IGNORE_FAIL(osMessageQueuePut(s_msg_queue, &msg, 0, 0));
#endif
		}
#endif
	}
}

void CANWrapper_Process_Message(CANMessage *msg)
{
	if (msg->is_ack)
	{
		// Search the TxCache to verify that this ACK corresponds to
		// something we sent. Delete the entry if it is.
		int index = TxCache_Find(&s_tx_cache, msg);
		TxCache_Erase(&s_tx_cache, index);
	}
#ifdef CWM_MODE_MANUAL
	// Pass all messages to the user callback.
	s_init_struct.message_callback(*msg);
#else
	else
	{
		// Acknowledge the received message.
		transmit_internal(msg->sender, msg, true);

		// Execute the command.
		s_init_struct.message_callback(*msg);
	}
#endif
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	// TODO
	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_ACK)
	{
		// timed out.
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_EWG)
	{
		// error warning. (96 errors recorded from transmission or receipt)
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_EPV)
	{
		// entered error passive state. (more than 16 failed transmission attempts and/or 128 failed receipts)
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_BOF)
	{
		// entered bus-off state. (more than 32 failed transmission attempts)
	}
}
