/** (c) 2024 UMSATS
 * @file msg_task.h
 *
 * RTOS task which delivers incoming messages to the message callback.
 */

#ifndef TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_MSG_TASK_H_
#define TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_MSG_TASK_H_

#ifdef CWM_DEFINE_RTOS_TASK

#include "can_wrapper.h"

#include "cmsis_os.h"

/**
 * @brief Initialises the message RTOS task.
 *
 * @param msg_queue     An RTOS queue to store messages.
 * @param callback      Callback for processing commands.
 */
void CANWrapper_Init_Message_Task(osMessageQueueId_t msg_queue, CANCommandHandlerCallback callback);

void CANWrapper_Start_Message_Task();

#endif

#endif /* TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_MSG_TASK_H_ */
