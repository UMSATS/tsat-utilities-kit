/** (c) 2024 UMSATS
 * @file cwm_mode.h
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_CAN_WRAPPER_CWM_MODE_H_
#define TSAT_UTILITIES_KIT_INC_TUK_CAN_WRAPPER_CWM_MODE_H_

#if defined(CAN_WRAPPER_MANUAL_MODE)
	#define CWM_MODE_MANUAL
#elif defined(CAN_WRAPPER_RTOS_MODE)
	#define CWM_MODE_RTOS
#else
	#define CWM_MODE_NORMAL
#endif

#endif /* TSAT_UTILITIES_KIT_INC_TUK_CAN_WRAPPER_CWM_MODE_H_ */
