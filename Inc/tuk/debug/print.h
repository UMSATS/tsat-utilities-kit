/** (c) 2024 UMSATS
 * @file print.h
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_DEBUG_PRINT_H_
#define TSAT_UTILITIES_KIT_INC_TUK_DEBUG_PRINT_H_

#include <stdio.h>

// disable printing in release mode.
#ifndef DEBUG
#define DISABLE_PRINTING
#endif

#ifndef DISABLE_PRINTING

	#define PRINT_INFO(...) \
		do { \
			printf("[" PRINT_SUBJECT "] " __VA_ARGS__); \
			printf("\n"); \
		} while (0)

	#define PRINT_ERROR(...) \
		do { \
			printf("[" PRINT_SUBJECT "] ERROR: " __VA_ARGS__); \
			printf(" ('%s':%d)\n", __FILE__, __LINE__); \
		} while (0)

	#define PRINT_WARN(...) \
		do { \
			printf("[" PRINT_SUBJECT "] WARNING: " __VA_ARGS__); \
			printf(" ('%s':%d)\n", __FILE__, __LINE__); \
		} while (0)

#else

	#define PRINT_INFO(...)((void)0U)
	#define PRINT_ERROR(...)((void)0U)
	#define PRINT_WARN(...)((void)0U)

#endif

#endif /* TSAT_UTILITIES_KIT_INC_TUK_DEBUG_PRINT_H_ */
