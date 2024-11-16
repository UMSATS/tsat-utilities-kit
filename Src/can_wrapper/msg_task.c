/** (c) 2024 UMSATS
 * @file msg_task.c
 *
 * RTOS task which delivers incoming messages to the message callback.
 */

#include "tuk/can_wrapper/cwm_mode.h"

#ifdef CWM_MODE_RTOS

#include "tuk/can_wrapper/msg_task.h"
#include "tuk/can_wrapper/can_message.h"
#include "tuk/can_wrapper/can_wrapper_status.h"

#include "cmsis_os.h"

// Message queue
static osMessageQueueId_t s_msg_queue;

// Message task
typedef StaticTask_t osStaticThreadDef_t;
static osStaticThreadDef_t s_msg_task_control_block;
static osThreadId_t s_msg_task;

static void start_msg_task();

// Forward declaration. Defined in can_wrapper.c
extern CANWrapper_StatusTypeDef CANWrapper_Process_Message(CANMessage *msg);

void CANWrapper_Message_Task_Init(const CANWrapper_Msg_Task_InitTypeDef *init_struct,
		osMessageQueueId_t msg_queue)
{
	s_msg_queue = msg_queue;
	const osThreadAttr_t msg_task_attributes = {
			.name = "CANMessageTask",
			.cb_mem = &s_msg_task_control_block,
			.cb_size = sizeof(s_msg_task_control_block),
			.stack_mem = init_struct->msg_task_stack_buffer,
			.stack_size = init_struct->msg_task_stack_size,
			.priority = init_struct->msg_task_priority,
	};
	s_msg_task = osThreadNew(&start_msg_task, NULL, &msg_task_attributes);
}

static void start_msg_task()
{
	CANMessage msg;

	// Infinite loop.
	while (1)
	{
		osMessageQueueGet(s_msg_queue, &msg, NULL, osWaitForever);
		CANWrapper_Process_Message(&msg);
	}
	osThreadExit();
}

#endif
