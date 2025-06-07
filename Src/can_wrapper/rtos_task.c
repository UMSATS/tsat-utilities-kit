/** (c) 2024 UMSATS
 * @file rtos_task.c
 *
 * RTOS task which invokes the command handler for each incoming command.
 */

#ifdef CWM_DEFINE_RTOS_TASK

#include "tuk/can_wrapper/rtos_task.h"
#include "tuk/can_wrapper/can_message.h"
#include "tuk/error_list.h"

#include "cmsis_os.h"

// Message queue
static osMessageQueueId_t s_msg_queue;

// Callback
static CANCommandHandlerCallback s_handler_callback;

void CANWrapper_Init_RTOS_Task(osMessageQueueId_t msg_queue, CANCommandHandlerCallback handler_callback)
{
	s_msg_queue = msg_queue;
	s_handler_callback = handler_callback;
}

void CANWrapper_Start_RTOS_Task()
{
	CANMessage msg;

	// Infinite loop.
	while (1)
	{
		// Wait for the next message to arrive.
		osMessageQueueGet(s_msg_queue, &msg, NULL, osWaitForever);

		if (msg->is_ack)
		{
			CANWrapper_Process_Ack(&msg);
		}
		else
		{
			CANWrapper_Transmit_Ack(&msg);

			// Pass it to the user callback.
			s_handler_callback(msg);
		}
	}
	osThreadExit();
}

#endif
