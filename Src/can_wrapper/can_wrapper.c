/**
 * @file can_wrapper.c
 * CAN wrapper for simplified message receipt & transmission.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 * @author Arnav Gupta <arnav.gupta@umsats.ca>
 *
 * @date February 28, 2024
 */

#include <stddef.h>

#include "tuk/can_wrapper/can_queue.h"
#include "tuk/can_wrapper/error_queue.h"
#include "tuk/can_wrapper/can_wrapper.h"
#include "tuk/can_wrapper/tx_cache.h"

#define ACK_MASK       0b00000000001
#define RECIPIENT_MASK 0b00000000110
#define SENDER_MASK    0b00000011000
#define PRIORITY_MASK  0b11111100000

#define TIMEOUT 3600

#define PERIOD_TICKS 5000

static CANWrapper_InitTypeDef s_init_struct = {0};

#ifndef CWM_IMMEDIATE_MODE
static CANQueue s_msg_queue = {0};
#endif

static TxCache s_tx_cache = {0};
static ErrorQueue s_error_queue = {0};

static bool s_init = false;

static CANWrapper_StatusTypeDef transmit_internal(NodeID recipient, CANMessage *msg, bool is_ack);

CANWrapper_StatusTypeDef CANWrapper_Init(CANWrapper_InitTypeDef init_struct)
{
	if ( !(init_struct.node_id <= 3
		&& init_struct.message_callback != NULL
		&& init_struct.hcan != NULL
		&& init_struct.htim != NULL)) // TODO
	{
		return CAN_WRAPPER_INVALID_ARGS;
	}

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

	if (HAL_CAN_ConfigFilter(init_struct.hcan, &filter_config) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_CONFIG_FILTER;
	}

	if (HAL_CAN_Start(init_struct.hcan) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_START_CAN;
	}

	// enable CAN interrupt.
	if (HAL_CAN_ActivateNotification(init_struct.hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_ENABLE_INTERRUPT;
	}

	if (HAL_TIM_Base_Start(init_struct.htim) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_START_TIMER;
	}

#ifndef CWM_IMMEDIATE_MODE
	s_msg_queue = CANQueue_Create();
#endif
	s_tx_cache = TxCache_Create();
	s_error_queue = ErrorQueue_Create();

	s_init_struct = init_struct;

	s_init = true;
	return CAN_WRAPPER_HAL_OK;
}

#ifndef CWM_IMMEDIATE_MODE
CANWrapper_StatusTypeDef CANWrapper_Poll_Messages()
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	// Poll incoming message queue.
	CANQueueItem queue_item;
	while (CANQueue_Dequeue(&s_msg_queue, &queue_item))
	{
		if (queue_item.msg.is_ack)
		{
			// check if this is an ACK for a transmitted message.
			int index = TxCache_Find(&s_tx_cache, &queue_item.msg);
			TxCache_Erase(&s_tx_cache, index); // delete it if it is.
		}

		if (!queue_item.msg.is_ack || s_init_struct.notify_of_acks)
		{
			s_init_struct.message_callback(queue_item.msg.msg, queue_item.msg.sender, queue_item.msg.is_ack);
		}
	}

	return CAN_WRAPPER_HAL_OK;
}
#endif

CANWrapper_StatusTypeDef CANWrapper_Poll_Errors()
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	// Poll transmission timeout events.
	// TODO: Needs serious cleaning up.
	uint32_t counter_value = __HAL_TIM_GET_COUNTER(s_init_struct.htim);
	uint32_t rcr_value = s_init_struct.htim->Instance->RCR;
	uint64_t current_tick = counter_value + rcr_value*PERIOD_TICKS;

	while (s_tx_cache.size > 0)
	{
		const TxCacheItem *front_item = &s_tx_cache.items[0];

		uint64_t tx_tick = front_item->timestamp.counter_value + front_item->timestamp.rcr_value*PERIOD_TICKS;
		uint64_t timeout_tick = (tx_tick + TIMEOUT) % (16*PERIOD_TICKS);

		bool clock_overflowed = tx_tick >= timeout_tick;
		bool timeout_occurred = clock_overflowed ?
				( current_tick >= timeout_tick && current_tick < tx_tick )
			: ( current_tick >= timeout_tick || current_tick < tx_tick );

		if (timeout_occurred)
		{
			CANWrapper_ErrorInfo error_info;
			error_info.error = CAN_WRAPPER_ERROR_TIMEOUT;
			error_info.msg = front_item->msg.msg;
			error_info.recipient = front_item->msg.recipient;
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

CANWrapper_StatusTypeDef CANWrapper_Transmit(NodeID recipient, CANMessage *msg)
{
	return transmit_internal(recipient, msg, false);
}

static CANWrapper_StatusTypeDef transmit_internal(NodeID recipient, CANMessage *msg, bool is_ack)
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	CmdConfig config = cmd_configs[msg->cmd];

	// TX message parameters.
	uint16_t id = (config.priority       << 5 & PRIORITY_MASK)
	            | (s_init_struct.node_id << 3 & SENDER_MASK)
	            | (recipient             << 1 & RECIPIENT_MASK)
				| (is_ack ? ACK_MASK : 0);

	// wait to send CAN message.
	while (HAL_CAN_GetTxMailboxesFreeLevel(s_init_struct.hcan) == 0){} // TODO: make sure this is safe. If not, limit with loop counter.

	CAN_TxHeaderTypeDef tx_header;
	tx_header.IDE = CAN_ID_STD;   // use standard identifier.
	tx_header.StdId = id;         // define standard identifier.
	tx_header.RTR = CAN_RTR_DATA; // specify as data frame.
	tx_header.DLC = 1 + config.body_size; // cmd ID + message body.

	if (!is_ack)
	{
		uint32_t counter_value = __HAL_TIM_GET_COUNTER(s_init_struct.htim);
		TxCacheItem cached_msg = {
				.timestamp = {
						.counter_value = counter_value,
						.rcr_value = s_init_struct.htim->Instance->RCR
				},
				.msg = {
						.msg = *msg,
						.priority = config.priority,
						.sender = s_init_struct.node_id,
						.recipient = recipient,
						.is_ack = is_ack,
				}
		};

		TxCache_Push_Back(&s_tx_cache, &cached_msg);
	}

	uint32_t tx_mailbox; // transmit mailbox.
	return HAL_CAN_AddTxMessage(s_init_struct.hcan, &tx_header, (uint8_t*)&msg, &tx_mailbox);
}

// called by HAL when a new CAN message is received and pending.
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan == s_init_struct.hcan)
	{
		HAL_StatusTypeDef status;

		CANQueueItem queue_item = {0};

		// get CAN message.
		CAN_RxHeaderTypeDef rx_header; // message header.
		status = HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, (uint8_t*)&queue_item.msg);

		if (status != HAL_OK)
			return; // in theory this should never happen. :p

		bool is_ack      = ACK_MASK & rx_header.StdId;
		NodeID recipient = (RECIPIENT_MASK & rx_header.StdId) >> 1;
		NodeID sender    = (SENDER_MASK & rx_header.StdId) >> 3;

		if (recipient == s_init_struct.node_id && sender != s_init_struct.node_id) // TODO: use CAN filtering instead.
		{
			if (!is_ack)
			{
				// respond with ACK.
				transmit_internal(sender, &queue_item.msg.msg, true);
			}

#ifdef CWM_IMMEDIATE_MODE
			// Author's Note: It is a little messy that I'm using the queue_item
			// variable when there is no queue in Immediate Mode. However, this
			// involves the fewest copy operations and #ifdef directives.
			if (is_ack)
			{
				// check if this is an ACK for a transmitted message.
				// This adds an O(n) operation to the ISR which isn't ideal.
				// Though, the code is highly optimised so it shouldn't be too
				// problematic. We could revisit this at a later date if needed.
				int index = TxCache_Find(&s_tx_cache, &queue_item.msg);
				TxCache_Erase(&s_tx_cache, index); // delete it if it is.
			}

			if (!is_ack || s_init_struct.notify_of_acks)
			{
				s_init_struct.message_callback(queue_item.msg.msg, sender, is_ack);
			}
#else
			queue_item.msg.sender = sender;
			queue_item.msg.is_ack = is_ack;

			CANQueue_Enqueue(&s_msg_queue, queue_item);
#endif
		}
	}
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
