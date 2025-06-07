/** (c) 2024 UMSATS
 * @file rtos_task.h
 *
 * RTOS task which delivers incoming messages to the message callback.
 */

#ifndef TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_RTOS_TASK_H_
#define TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_RTOS_TASK_H_

#ifdef CWM_DEFINE_RTOS_TASK

#include "can_wrapper.h"

#include "cmsis_os.h"

/**
 * @brief Initialises the CAN Wrapper RTOS task.
 *
 * @param msg_queue     An RTOS queue for storing messages.
 * @param callback      Callback for handling incoming commands.
 */
void CANWrapper_Init_RTOS_Task(osMessageQueueId_t msg_queue, CANCommandHandlerCallback handler_callback);

/**
 * @brief Runs the RTOS task. Pass this into osThreadNew().
 */
void CANWrapper_Start_RTOS_Task();

#endif

#endif /* TSAT_UTILITIES_KIT_SRC_CAN_WRAPPER_RTOS_TASK_H_ */
