cmake_minimum_required(VERSION 3.23)
project(tsat-utilities-kit)

set(CAN_WRAPPER_SRC_DIR Src/can_wrapper)

#assumes the source is of the form library_name.c
set(LIBRARIES "TxCache;CanCommandList")

foreach(library IN LISTS LIBRARIES)
    #coverts library name to source file name.
    #e.g. TxCache -> tx_cache.c, CANWrapper -> can_wrapper.c
    #p.s. I LOVE REGEX
    string(REGEX REPLACE "([A-Z]+)([A-Z])" "\\1_\\2" library_source "${library}.c")
    string(REGEX REPLACE "([a-z])([A-Z])" "\\1_\\2" library_source ${library_source})
    string(TOLOWER ${library_source} library_source)

    message(${library_source})
    add_library(${library})
    target_sources(${library}
        PUBLIC
        ${CAN_WRAPPER_SRC_DIR}/${library_source}
    )
    target_include_directories(${library} PUBLIC Inc)
endforeach()


#TODO for CANWrapper mock
#TYPES
#CAN_HandleTypeDef
#CAN_FilterTypeDef
#CAN_FILTER_FIFO0
#osThreadId_t
#osMessageQueueId_t
#osThreadAttr_t
#osFlagsWaitAny

#Functions
#osMessageQueueNew
#osThreadNew
#osMessageQueueGet
#osKernelGetTickFreq
#osThreadFlagsWait
#osDelayUntil

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(Tests)
endif()
