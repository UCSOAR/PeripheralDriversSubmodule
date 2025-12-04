 /**
 ********************************************************************************
 * @file    NAU7802Task.cpp
 * @author  Javier
 * @date    2025-11-25
 * @brief   Implementation of the NAU7802 task handling.
 ********************************************************************************
 */

 /************************************
 * INCLUDES
 ************************************/
#include "NAU7802Task.hpp"
#include "main.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"

 /************************************
 * MACROS AND DEFINES
 ************************************/
/* temp definitions if not SystemDefines.hpp*/
#ifndef NAU_TASK_QUEUE_DEPTH
#define NAU_TASK_QUEUE_DEPTH 5
#endif

#ifndef NAU_TASK_STACK_DEPTH
#define NAU_TASK_STACK_DEPTH 256 // Adjust..
#endif

#ifndef NAU_TASK_PRIORITY
#define NAU_TASK_PRIORITY 3
#endif

 /************************************
 * FUNCTION DEFINITIONS
 ************************************/

 // Constructor
 NAU7802Task::NAU7802Task() : Task(NAU_TASK_QUEUE_DEPTH)
{
    _i2c_wrapper = nullptr;
    _adc = nullptr;
}

void NAU7802Task::Init(I2C_HandleTypeDef* hi2c)
{
    SOAR_ASSERT(hi2c != nullptr, "NAU7802Task: Received Null I2C Handle");

    // Initialize objects for driver use
    _i2c_wrapper = new I2C_Wrapper(hi2c);
    //_adc = new NAU7802(_i2c_wrapper, HAL_Delay);
    _adc = new NAU7802(_i2c_wrapper, HAL_Delay);
    //_adc = new NAU7802(_i2c_wrapper, NauDelay(HAL_Delay));
}


void NAU7802Task::InitTask() // RTOS Task Init
{
    // Make sure dependencies are set
    SOAR_ASSERT(_adc != nullptr, "NAU7802Task: Driver not initialized. Call Init(hi2c) first.");

    // Assert Task not already created
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize NAU7802 task twice");
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)NAU7802Task::RunTask,
            (const char*)"NAU7802Task",
            (uint16_t)NAU_TASK_STACK_DEPTH,
            (void*)this,
            (UBaseType_t)NAU_TASK_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

     SOAR_ASSERT(rtValue == pdPASS, "NAU7802Task::InitTask() - xTaskCreate() failed");
}

void NAU7802Task::Run(void * pvParams)  // Instance Run loop for task
{
   /*   -Driver Setup-  */

   // Initialize 1x Gain
    if (_adc->begin(NAU7802_GAIN_1X) != NauStatus::OK){
        // Handle initialization error
        SOAR_PRINT("NAU7802Task: Sensor initialization failed.\n");
    } 
    else{
        SOAR_PRINT("NAU7802Task: Sensor initialized successfully.\n");
    }

    NAU7802_OUT adcData;

    /* ==Main Loop== */
    while (1)
    {
        // Check if new data is ready
        if (_adc->isReady()){
            // Read sensor data
            if (_adc->readSensor(&adcData) == NauStatus::OK){
                // SOAR_PRINT("NAU7802Task: ADC Reading: %ld\n", adc_data.raw_reading);
                
                // TODO: Send data somewhere
            }
            else{
                SOAR_PRINT("NAU7802Task: Failed to read sensor data.\n");
            }
        }

        // Handle incoming commands (optional)
        // receive with 0 timeout to poll
        Command cm;
        if (qEvtQueue->Receive(cm, 0)) {
            HandleCommand(cm);
        }
        
        // Yield / Delay to allow other tasks to run
        // TODO: Adjust delay as needed
        osDelay(10);
    }
}

void NAU7802Task::HandleCommand(Command & cm)
{
    // Handle task-specific commands here
    // Example:
    switch (cm.GetTaskCommand())
    {
    // TODO: Add command cases (Gain change, calibration, etc.)
    
    /*
    -- Example command --
    case 0: // 
    SOAR_PRINT("NAU7802Task: Received command 0.\n");
    break;
    */
    default:
        SOAR_PRINT("NAU7802Task: Received Unsupported Command {%d}.\n", cm.GetTaskCommand());
        break;
    }

    cm.Reset(); // Clean up command
}
