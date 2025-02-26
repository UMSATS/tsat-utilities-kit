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

void CANWrapper_Init_Message_Task(osMessageQueueId_t msg_queue);

void CANWrapper_Start_Message_Task();

#endif

#endif /* TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_MSG_TASK_H_ */
