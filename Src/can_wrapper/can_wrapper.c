/** (c) 2024 UMSATS
 * @file can_wrapper.c
 *
 * CAN wrapper for simplified CAN transmission & reception.
 */

#include "tuk/can_wrapper/can_wrapper.h"
#include "tuk/can_wrapper/can_queue.h"
#include "tuk/can_wrapper/error_queue.h"
#include "tuk/can_wrapper/tx_cache.h"
#include "tuk/debug.h"
#include "tuk/utils.h"

#include <stddef.h>

// Masks for StdId data fields
#define ACK_MASK       0b00000000001
#define RECIPIENT_MASK 0b00000000110
#define SENDER_MASK    0b00000011000
#define PRIORITY_MASK  0b11111100000

#define TIMEOUT 3600

#define PERIOD_TICKS 5000

#define QUEUE_SIZE 64

typedef struct
{
	CAN_HandleTypeDef *hcan;
	CANMessage msg;
} CANQueueItem;

static const CAN_FilterTypeDef FILTER_CONFIG = { // TODO: Look into how this works
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

static bool s_init = false;

static CANWrapper_InitTypeDef s_init_struct = {0};

static TxCache s_tx_cache = {0};

////////////////////////////////////////
/// RTOS Objects
////////////////////////////////////////
// Threads
static osThreadId_t s_ack_task;
static osThreadId_t s_msg_handler_task;
static osThreadId_t s_error_handler_task;
// Queues
static osMessageQueueId_t s_ack_queue;
static osMessageQueueId_t s_msg_queue;
static osMessageQueueId_t s_error_queue;

////////////////////////////////////////
/// Static Functions
////////////////////////////////////////
static void RTOS_Init();
static void Message_Handler_Thread(void *argument);
static void Acknowledgement_Thread(void *argument);
static void Error_Handler_Thread(void *argument);

ErrorCode CANWrapper_Init(const CANWrapper_InitTypeDef *init_struct)
{
#ifdef CWM_API_NORMAL
	ASSERT_PARAM(init_struct->node_id <= NODE_ID_MAX, ERR_ARG_OUT_OF_RANGE);
#endif
	ASSERT_PARAM(init_struct->message_callback != NULL, ERR_NULL_ARG);
	ASSERT_PARAM(init_struct->error_callback != NULL, ERR_NULL_ARG);
#ifdef CWM_API_ADVANCED
	ASSERT_PARAM(init_struct->rx_callback != NULL, ERR_NULL_ARG);
	ASSERT_PARAM(init_struct->tx_callback != NULL, ERR_NULL_ARG);
#endif

	s_init_struct = *init_struct;

	s_tx_cache = TxCache_Create();

	RTOS_Init();

	s_init = true;
	return ERR_OK;
}

/**
 * Creates all RTOS objects.
 */
void RTOS_Init()
{
	/* Acknowledgement Thread */
	s_ack_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(CANQueueItem), NULL);
	osThreadAttr_t ack_attr = {
			.name = "CAN ACK Thread",
			.stack_size = 128 * 4,
			.priority = (osPriority_t) osPriorityHigh
	};
	s_ack_task = osThreadNew(Acknowledgement_Thread, NULL, &ack_attr);

	/* Message Handler Thread */
	s_msg_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(CANQueueItem), NULL);
	osThreadAttr_t msg_handler_attr = {
			.name = "CAN Message Handler Thread",
			.stack_size = 128 * 4,
			.priority = (osPriority_t) osPriorityNormal
	};
	s_msg_handler_task = osThreadNew(Message_Handler_Thread, NULL, &msg_handler_attr);

	/* Error Handler Thread */
	s_error_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(CANQueueItem), NULL);
	osThreadAttr_t error_handler_attr = {
			.name = "CAN Error Handler Thread",
			.stack_size = 128 * 4,
			.priority = (osPriority_t) osPriorityNormal
	};
	s_msg_handler_task = osThreadNew(Error_Handler_Thread, NULL, &error_handler_attr);
}

ErrorCode CANWrapper_CAN_Start(CAN_HandleTypeDef *hcan)
{
	if (HAL_CAN_ConfigFilter(hcan, &FILTER_CONFIG) != HAL_OK)
	{
		return ERR_CWM_FAILED_TO_CONFIG_FILTER;
	}

	if (HAL_CAN_Start(hcan) != HAL_OK)
	{
		return ERR_CWM_FAILED_TO_START_CAN;
	}

	if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		return ERR_CWM_FAILED_TO_ENABLE_INTERRUPT;
	}

	return ERR_OK;
}

ErrorCode CANWrapper_Transmit_Raw(CAN_HandleTypeDef *hcan, const CANMessage *msg, bool strict_timeout)
{
	if (!s_init) return ERR_CWM_NOT_INITIALISED;

	if (HAL_CAN_GetState(hcan) != HAL_CAN_STATE_READY &&
			HAL_CAN_GetState(hcan) != HAL_CAN_STATE_LISTENING)
	{
		return ERR_CWM_TX_FAIL_BAD_CAN_STATE;
	}

	/* Define TX header */
	CAN_TxHeaderTypeDef tx_header = { 0 };
	tx_header.StdId = (msg->priority  << 5 & PRIORITY_MASK)
					| (msg->sender    << 3 & SENDER_MASK)
					| (msg->recipient << 1 & RECIPIENT_MASK)
					| (msg->is_ack         & ACK_MASK);
	tx_header.IDE = CAN_ID_STD; // Use standard identifier
	tx_header.RTR = CAN_RTR_DATA; // Specify data frame.
	tx_header.DLC = 1 + msg->body_size; // Length = cmd ID + body.

	/* Wait until mailboxes are free */
	uint16_t limiter = 0;
	while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0)
	{
		// Return if mailboxes aren't being freed.
		// For reasoning of the limit see:
		// https://github.com/UMSATS/tsat-utilities-kit/issues/31#issuecomment-2287744661
		if (limiter >= 4000) return ERR_CWM_TX_MAILBOXES_FULL;
		limiter++;
	}

	/* Load the message into a mailbox */
	uint32_t tx_mailbox;
	HAL_StatusTypeDef status;
	status = HAL_CAN_AddTxMessage(hcan, &tx_header, &msg->body, &tx_mailbox);

	/* Update the TX cache */
	if (status == HAL_OK && !msg->is_ack && strict_timeout)
	{
		uint32_t timestamp = osKernelGetTickCount();

		TxCacheItem tx_cache_item = {
				.timestamp = timestamp,
				.msg = { 0 }
		};
		memcpy(&tx_cache_item.msg, msg, sizeof(CANMessage));

		TxCache_Push_Back(&s_tx_cache, &tx_cache_item);

		osThreadFlagsSet(s_error_handler_task, 0x1U);
	}

#ifdef CWM_API_ADVANCED
	/* Call the TX callback */
	s_init_struct.tx_callback(hcan, msg);
#endif

	return status;
}

ErrorCode CANWrapper_Transmit(CAN_HandleTypeDef *hcan, NodeID recipient, CmdID cmd, const uint8_t *body)
{
	CANMessage msg = {
			.cmd = cmd,
			.body = { 0 },
			.body_size = CMD_CONFIGS[cmd].body_size,
			.priority = CMD_CONFIGS[cmd].priority,
			.sender = s_init_struct.node_id,
			.recipient = recipient,
			.is_ack = false
	};
	memcpy(msg.body, body, msg.body_size);

	return CANWrapper_Transmit_Raw(hcan, &msg, true);
}

////////////////////////////////////////
/// Threads
////////////////////////////////////////
void Message_Handler_Thread(void *argument)
{
	CANQueueItem item;

	// Infinite loop
	while (1)
	{
		// Wait for the next item in the queue.
		if (osMessageQueueGet(s_msg_queue, &item, NULL, osWaitForever) == osOK)
		{
			// Let the user handle it.
			s_init_struct.message_callback(item.hcan, &item.msg);
		}
	}
}

void Acknowledgement_Thread(void *argument)
{
	CANQueueItem item;

	// Infinite loop
	while (1)
	{
		// Wait for the next item in the queue.
		if (osMessageQueueGet(s_ack_queue, &item, NULL, osWaitForever) == osOK)
		{
			CANWrapper_Transmit_Raw(item.hcan, &item.msg, false);
		}
	}
}

void Error_Handler_Thread(void *argument)
{
	CANWrapper_ErrorInfo item;

	// Infinite loop
	while (1)
	{
		// Wait for a message in the Tx cache.
		if (s_tx_cache.size == 0)
		{
			osThreadFlagsWait(0U, osFlagsWaitAny, osWaitForever);
		}

		// Calculate timeout tick
		// (assuming TIMEOUT is in milliseconds and RTOS ticks are 1 KHz)
		const uint32_t timestamp = s_tx_cache.items[0].timestamp;
		const uint32_t timeout_tick = timestamp + TIMEOUT;

		// Wait for the message to time out.
		osDelayUntil(timeout_tick);

		// Trigger user callback if the same message is still in the cache
		// (comparing timestamps to reduce read instructions).
		if (timestamp == s_tx_cache.items[0].timestamp)
		{
			CANWrapper_ErrorInfo error;
			error.error = CAN_WRAPPER_ERROR_TIMEOUT;
			error.msg = s_tx_cache.items[0].msg;

			TxCache_Erase(&s_tx_cache, 0);

			s_init_struct.error_callback(&error);
		}
	}
}

////////////////////////////////////////
/// Interrupt Service Routines
////////////////////////////////////////
/**
 * Called when a CAN message is pending.
 *
 * @param hcan The source CAN peripheral.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_StatusTypeDef status;

	CANQueueItem item = {
			.hcan = hcan,
			.msg = { 0 }
	};

	// Retrieve the message header and body.
	CAN_RxHeaderTypeDef rx_header;
	status = HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, (uint8_t*)&item.msg.cmd);

	if (status != HAL_OK)
		return; // In theory this should never happen. :p

	if (rx_header.RTR != CAN_RTR_DATA)
		return; // Only handle data frames for now.

	// Extract message data from the header.
	item.msg.body_size = rx_header.DLC - 1;
	item.msg.priority  = (PRIORITY_MASK & rx_header.StdId) >> 5;
	item.msg.sender    = (SENDER_MASK & rx_header.StdId) >> 3;
	item.msg.recipient = (RECIPIENT_MASK & rx_header.StdId) >> 1;
	item.msg.is_ack    = (ACK_MASK & rx_header.StdId);

	// Define what actions to take in response to this message.
	uint8_t rx_behaviour = 0;

#ifdef CWM_API_NORMAL
	if (item.msg.recipient == s_init_struct.node_id &&
			item.msg.sender != s_init_struct.node_id)
	{
		rx_behaviour = RX_ACK | RX_CLEAR_TX_STORE;
		if (!item.msg.is_ack)
		{
			rx_behaviour |= RX_HANDLE;
		}
	}
#elif defined(CWM_API_ADVANCED)
	// Let the user define RX behaviour.
	s_init_struct.rx_callback(item.hcan, &item.msg, &rx_behaviour);
#endif
	if (rx_behaviour | RX_ACK && !item.msg.is_ack)
	{
		// Create an ACK message.
		CANQueueItem ack;
		memcpy(&ack, &item, sizeof(CANQueueItem));
		ack.msg.recipient = item.msg.sender;
		ack.msg.sender = item.msg.recipient;
		ack.msg.is_ack = true;
		osMessageQueuePut(&s_ack_queue, &ack, 0U, 0U);
	}
	if (rx_behaviour | RX_HANDLE)
	{
		osMessageQueuePut(&s_msg_queue, &item, 0U, 0U);
	}
	if (rx_behaviour | RX_CLEAR_TX_STORE && item.msg.is_ack)
	{
		// Clear the corresponding message in the TxCache if it exists.
		int index = TxCache_Find(&s_tx_cache, &item.msg);
		if (index > 0)
		{
			TxCache_Erase(&s_tx_cache, index);
		}
	}
}

/**
 * Called when a CAN peripheral raises an error.
 *
 * @param hcan The source CAN peripheral.
 */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) // TODO
{
	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_ACK)
	{
		// Timed out.
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_EWG)
	{
		// Error warning. (96 errors recorded from transmission or reception)
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_EPV)
	{
		// Entered error passive state. (more than 16 failed transmission attempts and/or 128 failed receptions)
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_BOF)
	{
		// Entered bus-off state. (more than 32 failed transmission attempts)
	}
}
