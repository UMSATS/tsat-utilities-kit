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

// Forward declaration. Defined in can_wrapper.c
extern CANWrapper_StatusTypeDef CANWrapper_Process_Message(CANMessage *msg);

void CANWrapper_Init_Message_Task(osMessageQueueId_t msg_queue)
{
	s_msg_queue = msg_queue;
}

void CANWrapper_Start_Message_Task()
{
	CANMessage msg;

	// Infinite loop.
	while (1)
	{
		// Wait for the next message to arrive.
		osMessageQueueGet(s_msg_queue, &msg, NULL, osWaitForever);

		// Pass it to the user code.
		CANWrapper_Process_Message(&msg);
	}
	osThreadExit();
}

#endif
