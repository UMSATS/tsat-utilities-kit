/** (c) 2024 UMSATS
 * @file msg_task.c
 *
 * RTOS task which delivers incoming messages to the message callback.
 */

#ifdef CWM_DEFINE_RTOS_TASK

#include "tuk/can_wrapper/msg_task.h"
#include "tuk/can_wrapper/can_message.h"
#include "tuk/error_list.h"

#include "cmsis_os.h"

// Message queue
static osMessageQueueId_t s_msg_queue;

// Callback
static CANCommandHandlerCallback s_handler_callback;

void CANWrapper_Init_Message_Task(osMessageQueueId_t msg_queue, CANCommandHandlerCallback handler_callback)
{
	s_msg_queue = msg_queue;
	s_handler_callback = handler_callback;
}

void CANWrapper_Start_Message_Task()
{
	CANMessage msg;

	// Infinite loop.
	while (1)
	{
		// Wait for the next message to arrive.
		osMessageQueueGet(s_msg_queue, &msg, NULL, osWaitForever);

		// Pass it to the user callback.
		s_handler_callback(msg);
	}
	osThreadExit();
}

#endif
