/** (c) 2024 UMSATS
 * @file msg_task.h
 *
 * RTOS task which delivers incoming messages to the message callback.
 */

#ifndef TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_MSG_TASK_H_
#define TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_MSG_TASK_H_

#include "tuk/can_wrapper/cwm_mode.h"

#ifdef CWM_MODE_RTOS

#include "cmsis_os.h"

typedef struct
{
	osPriority msg_task_priority;    // The priority to assign the message task.
	uint32_t *msg_task_stack_buffer; // A buffer for the message task stack.
	size_t msg_task_stack_size;      // The size of the stack buffer in bytes.
} CANWrapper_Msg_Task_InitTypeDef;

void CANWrapper_Message_Task_Init(const CANWrapper_Msg_Task_InitTypeDef *init_struct, osMessageQueueId_t msg_queue);

#endif

#endif /* TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_MSG_TASK_H_ */
