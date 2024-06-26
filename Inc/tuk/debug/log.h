/**
 * @file log.h
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date May 27, 2024
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_DEBUG_LOG_H_
#define TSAT_UTILITIES_KIT_INC_TUK_DEBUG_LOG_H_

#include <stdio.h>

#ifndef DISABLE_LOGGING

	#define LOG_INFO(...) \
		do { \
			printf("[" LOG_SUBJECT "] " __VA_ARGS__); \
			printf("\n"); \
		} while (0)

	#define LOG_ERROR(...) \
		do { \
			fprintf(stderr, "[" LOG_SUBJECT "] Error: " __VA_ARGS__); \
			fprintf(stderr, " ('%s':%d)\n", __FILE__, __LINE__); \
		} while (0)

	#define LOG_WARN(...) \
		do { \
			printf("[" LOG_SUBJECT "] Warning: " __VA_ARGS__); \
			printf(" ('%s':%d)\n", __FILE__, __LINE__); \
		} while (0)

#else

	#define LOG_INFO(...)((void)0U)
	#define LOG_ERROR(...)((void)0U)
	#define LOG_WARN(...)((void)0U)

#endif

#endif /* TSAT_UTILITIES_KIT_INC_TUK_DEBUG_LOG_H_ */
