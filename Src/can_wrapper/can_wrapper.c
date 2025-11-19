/** (c) 2024 UMSATS
 * @file can_wrapper.c
 *
 * CAN wrapper for simplified CAN transmission & reception.
 * IMPROVED VERSION: Single ownership pattern eliminates race conditions.
 *
 * THREAD SAFETY:
 * ==============
 * This module uses a single-owner pattern for race-condition-free operation:
 *
 * SHARED STATE:
 * - s_init:         Read by multiple threads. Must be set AFTER full init completes.
 * - s_init_struct:  Written once in Init, read by threads/ISR. Init must complete
 *                   before osKernelStart().
 * - s_tx_cache:     SINGLE OWNER: Cache_Manager_Thread. All other threads/ISR
 *                   communicate via commands in s_cache_command_queue.
 * - s_stats:        Updated by multiple threads. Individual counters are best-effort
 *                   (may lose counts under high contention, but this is acceptable for
 *                   diagnostics).
 *
 * RTOS OBJECTS (read-only after init):
 * - s_ack_queue, s_msg_queue, s_cache_command_queue, s_error_notification_queue
 * - s_ack_task, s_msg_handler_task, s_cache_manager_task, s_error_callback_task
 *
 * INITIALIZATION REQUIREMENTS:
 * - CANWrapper_Init() must complete before osKernelStart()
 * - No other CAN wrapper functions should be called during Init
 * - After Init completes, all operations are thread-safe and ISR-safe
 *
 * ISR SAFETY:
 * - HAL_CAN_RxFifo0MsgPendingCallback runs in ISR context
 * - Only queues operations, never blocks
 * - All osMessageQueuePut calls use 0 timeout
 * - Queue overflows are tracked in statistics
 *
 * QUEUE FULL BEHAVIOR:
 * - If cache_command_queue is full: TX succeeds but timeout tracking may fail
 * - If ack_queue is full: ACK is lost (tracked in stats)
 * - If msg_queue is full: Message is lost (tracked in stats)
 * - If error_queue is full: Error callback is lost (tracked in stats)
 * - All overflows are counted for telemetry/debugging
 */

#include "../../Inc/tuk/can_wrapper/can_wrapper.h"
#include "../../Inc/tuk/can_wrapper/error_info.h"
#include "../../Inc/tuk/can_wrapper/tx_cache.h"
#include "../../Inc/tuk/debug/debug.h"
#include "../../Inc/tuk/debug/utils.h"

#include <stddef.h>

// Masks for StdId data fields
#define ACK_MASK       0b00000000001
#define RECIPIENT_MASK 0b00000000110
#define SENDER_MASK    0b00000011000
#define PRIORITY_MASK  0b11111100000

#define TIMEOUT_MS 4
#define QUEUE_SIZE 64

// Cache operation commands
typedef enum {
    CACHE_CMD_ADD,
    CACHE_CMD_REMOVE_BY_ACK,
    CACHE_CMD_SHUTDOWN
} CacheCommandType;

typedef struct {
    CacheCommandType type;
    CANMessage msg;
    uint32_t timestamp;  // Used for ADD operations
} CacheCommand;

// Error notification for callback thread
typedef struct {
    CANWrapper_ErrorType error;
    CANMessage msg;
} ErrorNotification;

typedef struct {
    CAN_HandleTypeDef *hcan;
    CANMessage msg;
} CANQueueItem;

static const CAN_FilterTypeDef FILTER_CONFIG = {
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
/// Diagnostic Statistics
////////////////////////////////////////
typedef struct {
	uint32_t cache_queue_overflows;
	uint32_t ack_queue_overflows;
	uint32_t msg_queue_overflows;
	uint32_t error_queue_overflows;
	uint32_t messages_transmitted;
	uint32_t messages_received;
	uint32_t acks_sent;
	uint32_t timeouts;
} CANWrapper_Statistics;

static CANWrapper_Statistics s_stats = {0};

////////////////////////////////////////
/// RTOS Objects
////////////////////////////////////////
// Threads
static osThreadId_t s_ack_task;
static osThreadId_t s_msg_handler_task;
static osThreadId_t s_cache_manager_task;
static osThreadId_t s_error_callback_task;

// Queues
static osMessageQueueId_t s_ack_queue;
static osMessageQueueId_t s_msg_queue;
static osMessageQueueId_t s_cache_command_queue;
static osMessageQueueId_t s_error_notification_queue;

////////////////////////////////////////
/// Static Functions
////////////////////////////////////////
static ErrorCode RTOS_Init(void);
static void Message_Handler_Thread(void *argument);
static void Acknowledgement_Thread(void *argument);
static void Cache_Manager_Thread(void *argument);
static void Error_Callback_Thread(void *argument);
static void Process_Expired_Items(void);

ErrorCode CANWrapper_Init(const CANWrapper_InitTypeDef *init_struct)
{
#ifdef CWM_API_STANDARD
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

    ErrorCode rtos_status = RTOS_Init();
    if (rtos_status != ERR_OK)
    {
        return rtos_status;
    }

    s_init = true;
    return ERR_OK;
}

/**
 * Creates all RTOS objects with validation.
 *
 * @return ERR_OK on success, error code on failure
 */
ErrorCode RTOS_Init(void)
{
    /* Acknowledgement Queue */
    s_ack_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(CANQueueItem), NULL);
    if (s_ack_queue == NULL)
    {
        return ERR_CWM_RTOS_QUEUE_CREATE_FAILED;
    }

    /* Acknowledgement Thread */
    osThreadAttr_t ack_attr = {
        .name = "CAN ACK Thread",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityHigh
    };
    s_ack_task = osThreadNew(Acknowledgement_Thread, NULL, &ack_attr);
    if (s_ack_task == NULL)
    {
        return ERR_CWM_RTOS_THREAD_CREATE_FAILED;
    }

    /* Message Handler Queue */
    s_msg_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(CANQueueItem), NULL);
    if (s_msg_queue == NULL)
    {
        return ERR_CWM_RTOS_QUEUE_CREATE_FAILED;
    }

    /* Message Handler Thread */
    osThreadAttr_t msg_handler_attr = {
        .name = "CAN Message Handler Thread",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityNormal
    };
    s_msg_handler_task = osThreadNew(Message_Handler_Thread, NULL, &msg_handler_attr);
    if (s_msg_handler_task == NULL)
    {
        return ERR_CWM_RTOS_THREAD_CREATE_FAILED;
    }

    /* Cache Command Queue */
    s_cache_command_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(CacheCommand), NULL);
    if (s_cache_command_queue == NULL)
    {
        return ERR_CWM_RTOS_QUEUE_CREATE_FAILED;
    }

    /* Cache Manager Thread - SINGLE OWNER of tx_cache */
    osThreadAttr_t cache_manager_attr = {
        .name = "CAN Cache Manager Thread",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityHigh  // High priority for timely processing
    };
    s_cache_manager_task = osThreadNew(Cache_Manager_Thread, NULL, &cache_manager_attr);
    if (s_cache_manager_task == NULL)
    {
        return ERR_CWM_RTOS_THREAD_CREATE_FAILED;
    }

    /* Error Notification Queue */
    s_error_notification_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(ErrorNotification), NULL);
    if (s_error_notification_queue == NULL)
    {
        return ERR_CWM_RTOS_QUEUE_CREATE_FAILED;
    }

    /* Error Callback Thread */
    osThreadAttr_t error_callback_attr = {
        .name = "CAN Error Callback Thread",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityNormal
    };
    s_error_callback_task = osThreadNew(Error_Callback_Thread, NULL, &error_callback_attr);
    if (s_error_callback_task == NULL)
    {
        return ERR_CWM_RTOS_THREAD_CREATE_FAILED;
    }

    return ERR_OK;
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
    tx_header.StdId = ((msg->priority  << 5) & PRIORITY_MASK)
                    | ((msg->sender    << 3) & SENDER_MASK)
                    | ((msg->recipient << 1) & RECIPIENT_MASK)
                    | (msg->is_ack          & ACK_MASK);
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 1 + msg->body_size;

    /* Wait until mailboxes are free */
    uint16_t limiter = 0;
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0)
    {
        if (limiter >= 4000) return ERR_CWM_TX_MAILBOXES_FULL;
        limiter++;
    }

    /* Load the message into a mailbox */
    uint32_t tx_mailbox;
    HAL_StatusTypeDef status;
    status = HAL_CAN_AddTxMessage(hcan, &tx_header, msg->body, &tx_mailbox);

    /* Add to cache via command (no direct access) */
    if (status == HAL_OK && !msg->is_ack && strict_timeout)
    {
        CacheCommand cmd = {
            .type = CACHE_CMD_ADD,
            .msg = *msg,
            .timestamp = osKernelGetTickCount()
        };

        // Queue command to cache manager
        osStatus_t queue_status = osMessageQueuePut(s_cache_command_queue, &cmd, 0U, 0U);
        if (queue_status != osOK)
        {
            // Queue full - timeout tracking will fail, but TX succeeded
            s_stats.cache_queue_overflows++;
            // Note: We continue anyway as TX was successful
        }
        else
        {
            s_stats.messages_transmitted++;
        }
    }
    else if (status == HAL_OK)
    {
        s_stats.messages_transmitted++;
    }

#ifdef CWM_API_ADVANCED
    /* Call the TX callback */
    s_init_struct.tx_callback(hcan, msg);
#endif

    return status;
}

#ifdef CWM_API_STANDARD
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
#endif

////////////////////////////////////////
/// Threads
////////////////////////////////////////
void Message_Handler_Thread(void *argument)
{
    CANQueueItem item;

    while (1)
    {
        if (osMessageQueueGet(s_msg_queue, &item, NULL, osWaitForever) == osOK)
        {
            s_init_struct.message_callback(item.hcan, &item.msg);
        }
    }
}

void Acknowledgement_Thread(void *argument)
{
    CANQueueItem item;

    while (1)
    {
        if (osMessageQueueGet(s_ack_queue, &item, NULL, osWaitForever) == osOK)
        {
            CANWrapper_Transmit_Raw(item.hcan, &item.msg, false);
        }
    }
}

/**
 * Cache Manager Thread - SINGLE OWNER of s_tx_cache
 * This thread is the ONLY one that directly accesses s_tx_cache.
 * All other operations go through commands.
 */
void Cache_Manager_Thread(void *argument)
{
    CacheCommand cmd;
    const uint32_t TICK_FREQ = osKernelGetTickFreq();
    const uint32_t TIMEOUT_TICKS = (uint32_t)TIMEOUT_MS * TICK_FREQ / 1000;

    while (1)
    {
        // Calculate next timeout (if any items exist)
        uint32_t next_timeout = osWaitForever;
        if (s_tx_cache.size > 0)
        {
            uint32_t oldest_timestamp = s_tx_cache.items[0].timestamp;
            uint32_t timeout_time = oldest_timestamp + TIMEOUT_TICKS;
            uint32_t current_time = osKernelGetTickCount();
            
            if (timeout_time <= current_time)
            {
                next_timeout = 0;  // Already expired
            }
            else
            {
                next_timeout = timeout_time - current_time;
            }
        }

        // Wait for either a command OR timeout
        osStatus_t status = osMessageQueueGet(s_cache_command_queue, &cmd, NULL, next_timeout);

        if (status == osOK)
        {
            // Process command
            switch (cmd.type)
            {
                case CACHE_CMD_ADD:
                {
                    TxCacheItem cache_item = {
                        .timestamp = cmd.timestamp,
                        .msg = cmd.msg
                    };
                    TxCache_Push_Back(&s_tx_cache, &cache_item);
                    break;
                }
                case CACHE_CMD_REMOVE_BY_ACK:
                {
                    int index = TxCache_Find(&s_tx_cache, &cmd.msg);
                    if (index >= 0)  // Fixed: was > 0, should be >= 0
                    {
                        TxCache_Erase(&s_tx_cache, index);
                    }
                    break;
                }
                case CACHE_CMD_SHUTDOWN:
                {
                    // Clean shutdown
                    return;
                }
            }
        }
        else if (status == osErrorTimeout)
        {
            // Process expired items
            Process_Expired_Items();
        }
    }
}

/**
 * Error Callback Thread - Executes user error callbacks
 * This prevents user callbacks from blocking cache operations.
 */
void Error_Callback_Thread(void *argument)
{
    ErrorNotification error;

    while (1)
    {
        if (osMessageQueueGet(s_error_notification_queue, &error, NULL, osWaitForever) == osOK)
        {
            // Execute user callback
            CANWrapper_ErrorInfo error_info = {
                .error = error.error,
                .msg = error.msg
            };
            s_init_struct.error_callback(&error_info);
        }
    }
}

/**
 * Process expired items in the cache.
 * Called only by Cache_Manager_Thread.
 */
static void Process_Expired_Items(void)
{
    const uint32_t TICK_FREQ = osKernelGetTickFreq();
    const uint32_t TIMEOUT_TICKS = (uint32_t)TIMEOUT_MS * TICK_FREQ / 1000;
    uint32_t current_time = osKernelGetTickCount();

    // Process all expired items (cache is sorted by timestamp)
    while (s_tx_cache.size > 0)
    {
        // Use TxCache_At for safe access (better encapsulation)
        const TxCacheItem *oldest_item = TxCache_At(&s_tx_cache, 0);
        if (oldest_item == NULL)
        {
            break;  // Safety check - should never happen
        }

        uint32_t item_timeout = oldest_item->timestamp + TIMEOUT_TICKS;

        if (item_timeout <= current_time)
        {
            // Item has expired - queue error notification
            ErrorNotification error = {
                .error = CAN_WRAPPER_ERROR_TIMEOUT,
                .msg = oldest_item->msg
            };

            // Queue error notification (non-blocking)
            osStatus_t status = osMessageQueuePut(s_error_notification_queue, &error, 0U, 0U);
            if (status != osOK)
            {
                // Error queue full - error callback will be lost
                s_stats.error_queue_overflows++;
            }

            // Track timeout
            s_stats.timeouts++;

            // Remove expired item
            TxCache_Erase(&s_tx_cache, 0);
        }
        else
        {
            break;  // No more expired items
        }
    }
}

////////////////////////////////////////
/// Interrupt Service Routines
////////////////////////////////////////
/**
 * Called when a CAN message is pending.
 * ISR now only queues operations - no direct cache access.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_StatusTypeDef status;

    CANQueueItem item = {
        .hcan = hcan,
        .msg = { 0 }
    };

    // Retrieve the message header and body
    CAN_RxHeaderTypeDef rx_header;
    status = HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, (uint8_t*)&item.msg.cmd);

    if (status != HAL_OK)
        return;

    if (rx_header.RTR != CAN_RTR_DATA)
        return;

    // Extract message data from the header
    item.msg.body_size = rx_header.DLC - 1;
    item.msg.priority  = (PRIORITY_MASK & rx_header.StdId) >> 5;
    item.msg.sender    = (SENDER_MASK & rx_header.StdId) >> 3;
    item.msg.recipient = (RECIPIENT_MASK & rx_header.StdId) >> 1;
    item.msg.is_ack    = (ACK_MASK & rx_header.StdId);

    // Define what actions to take in response to this message
    uint8_t rx_behaviour = 0;

#ifdef CWM_API_STANDARD
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
    s_init_struct.rx_callback(item.hcan, &item.msg, &rx_behaviour);
#endif

    // Track received message
    s_stats.messages_received++;

    // Fixed: Changed | to & for proper bit checking
    if ((rx_behaviour & RX_ACK) && !item.msg.is_ack)
    {
        // Create an ACK message
        CANQueueItem ack;
        memcpy(&ack, &item, sizeof(CANQueueItem));
        ack.msg.recipient = item.msg.sender;
        ack.msg.sender = item.msg.recipient;
        ack.msg.is_ack = true;

        osStatus_t status = osMessageQueuePut(s_ack_queue, &ack, 0U, 0U);
        if (status == osOK)
        {
            s_stats.acks_sent++;
        }
        else
        {
            // ACK queue full - ACK will be lost
            s_stats.ack_queue_overflows++;
        }
    }

    if (rx_behaviour & RX_HANDLE)
    {
        osStatus_t status = osMessageQueuePut(s_msg_queue, &item, 0U, 0U);
        if (status != osOK)
        {
            // Message queue full - message will be lost
            s_stats.msg_queue_overflows++;
        }
    }

    if ((rx_behaviour & RX_CLEAR_TX_STORE) && item.msg.is_ack)
    {
        // Send remove command to cache manager instead of direct access
        CacheCommand cmd = {
            .type = CACHE_CMD_REMOVE_BY_ACK,
            .msg = item.msg
        };

        osStatus_t status = osMessageQueuePut(s_cache_command_queue, &cmd, 0U, 0U);
        if (status != osOK)
        {
            // Cache command queue full - message will not be removed from cache
            // This could lead to spurious timeouts
            s_stats.cache_queue_overflows++;
        }
    }
}

/**
 * Called when a CAN peripheral raises an error.
 */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    // TODO: Implement proper error handling
    if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_ACK)
    {
        // Timed out
    }

    if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_EWG)
    {
        // Error warning (96 errors recorded)
    }

    if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_EPV)
    {
        // Entered error passive state
    }

    if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_BOF)
    {
        // Entered bus-off state
    }
}

////////////////////////////////////////
/// Cleanup Function (Optional)
////////////////////////////////////////
/**
 * Clean shutdown of the CAN wrapper.
 * Sends shutdown command to cache manager.
 */
ErrorCode CANWrapper_Shutdown()
{
    if (!s_init) return ERR_CWM_NOT_INITIALISED;
    
    CacheCommand cmd = {
        .type = CACHE_CMD_SHUTDOWN
    };
    
    return osMessageQueuePut(s_cache_command_queue, &cmd, 0U, 100);  // 100ms timeout
}
