/** (c) 2024 UMSATS
 * @file assert.h
 *
 * Defines assertion macros.
 */

#ifndef TSAT_UTILITIES_KIT_INC_TUK_DEBUG_ASSERT_H_
#define TSAT_UTILITIES_KIT_INC_TUK_DEBUG_ASSERT_H_

#ifdef USE_FULL_ASSERT

	#define ASSERT(cond) \
		do { \
			if (!(cond)) \
			{ \
				printf("[Assert] Assertion failed: '" #cond "' (%s:%i)\n", __FILE__, __LINE__); \
				assertion_failed(); \
			} \
		} while (0)


	#define ASSERT_PARAM(cond, err) \
		do { \
			if (!(cond)) \
			{ \
				printf("[Assert] Parameter assertion failed: '" #cond "' (%s:%i)\n", __FILE__, __LINE__); \
				assertion_failed(); \
				return err; \
			} \
		} while (0)

#else

	#define ASSERT(cond)((void)0U)

	#define ASSERT_PARAM(cond, err) \
		do { \
			if (!(cond)) \
			{ \
				printf("[Assert] Parameter assertion failed: '" #cond "' (%s:%i)\n", __FILE__, __LINE__); \
				return err; \
			} \
		} while (0)

#endif

void assertion_failed();

#endif /* TSAT_UTILITIES_KIT_INC_TUK_DEBUG_ASSERT_H_ */
