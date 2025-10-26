#include "../../Inc/tuk/can_wrapper/can_wrapper.h"

#ifdef RUN_TESTS
#include "test_can_wrapper.h"
#endif

int main(void)
{
    // Your existing initialization
    HAL_Init();
    SystemClock_Config();
    
    #ifdef RUN_TESTS
    // Run tests before normal operation
    printf("Starting CAN Wrapper Tests...\n");
    if (run_all_tests() != 0) {
        printf("TESTS FAILED - Stopping execution\n");
        while(1) {
            // Flash error LED or send debug info
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Error LED
            HAL_Delay(200);
        }
    }
    printf("ALL TESTS PASSED - Starting normal operation\n");
    #endif
    
}
