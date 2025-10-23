/** (c) 2024 UMSATS
 * @file can_wrapper.c
 *
 * CAN wrapper for simplified CAN transmission & reception.
 * IMPROVED VERSION: Single ownership pattern eliminates race conditions.
 */

#include "tuk/can_wrapper/can_wrapper.h"
#include "tuk/can_wrapper/error_info.h"
#include "tuk/can_wrapper/tx_cache.h"
#include "tuk/debug.h"
#include "tuk/utils.h"

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
static void RTOS_Init();
static void Message_Handler_Thread(void *argument);
static void Acknowledgement_Thread(void *argument);
static void Cache_Manager_Thread(void *argument);
static void Error_Callback_Thread(void *argument);
static uint32_t Calculate_Next_Timeout();
static void Process_Expired_Items();

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

    /* Cache Manager Thread - SINGLE OWNER of tx_cache */
    s_cache_command_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(CacheCommand), NULL);
    osThreadAttr_t cache_manager_attr = {
        .name = "CAN Cache Manager Thread",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityHigh  // High priority for timely processing
    };
    s_cache_manager_task = osThreadNew(Cache_Manager_Thread, NULL, &cache_manager_attr);

    /* Error Callback Thread */
    s_error_notification_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(ErrorNotification), NULL);
    osThreadAttr_t error_callback_attr = {
        .name = "CAN Error Callback Thread",
        .stack_size = 128 * 4,
        .priority = (osPriority_t) osPriorityNormal
    };
    s_error_callback_task = osThreadNew(Error_Callback_Thread, NULL, &error_callback_attr);
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
        // Note: We don't check return value here as it's better to continue
        // even if cache tracking fails
        osMessageQueuePut(s_cache_command_queue, &cmd, 0U, 0U);
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
static void Process_Expired_Items()
{
    const uint32_t TICK_FREQ = osKernelGetTickFreq();
    const uint32_t TIMEOUT_TICKS = (uint32_t)TIMEOUT_MS * TICK_FREQ / 1000;
    uint32_t current_time = osKernelGetTickCount();

    // Process all expired items (cache is sorted by timestamp)
    while (s_tx_cache.size > 0)
    {
        uint32_t item_timeout = s_tx_cache.items[0].timestamp + TIMEOUT_TICKS;
        
        if (item_timeout <= current_time)
        {
            // Item has expired - queue error notification
            ErrorNotification error = {
                .error = CAN_WRAPPER_ERROR_TIMEOUT,
                .msg = s_tx_cache.items[0].msg
            };
            
            // Queue error notification (non-blocking)
            osMessageQueuePut(s_error_notification_queue, &error, 0U, 0U);
            
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

    // Fixed: Changed | to & for proper bit checking
    if ((rx_behaviour & RX_ACK) && !item.msg.is_ack)
    {
        // Create an ACK message
        CANQueueItem ack;
        memcpy(&ack, &item, sizeof(CANQueueItem));
        ack.msg.recipient = item.msg.sender;
        ack.msg.sender = item.msg.recipient;
        ack.msg.is_ack = true;
        osMessageQueuePut(s_ack_queue, &ack, 0U, 0U);
    }
    
    if (rx_behaviour & RX_HANDLE)
    {
        osMessageQueuePut(s_msg_queue, &item, 0U, 0U);
    }
    
    if ((rx_behaviour & RX_CLEAR_TX_STORE) && item.msg.is_ack)
    {
        // Send remove command to cache manager instead of direct access
        CacheCommand cmd = {
            .type = CACHE_CMD_REMOVE_BY_ACK,
            .msg = item.msg
        };
        osMessageQueuePut(s_cache_command_queue, &cmd, 0U, 0U);
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
