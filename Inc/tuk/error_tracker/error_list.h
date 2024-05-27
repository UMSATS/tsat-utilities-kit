/**
 * @file error_list.h
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date May 27, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_LIST_H_
#define TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_LIST_H_

typedef enum {
	// COMMON
#include "errors/common.txt"

	// CDH
#include "errors/cdh.txt"

	// POWER
#include "errors/power.txt"

	// ADCS
#include "errors/adcs.txt"

	// PAYLOAD
#include "errors/payload.txt"
} ErrorID;

#endif /* TSAT_UTILITIES_KIT_INC_TUK_ERROR_TRACKER_ERROR_LIST_H_ */
