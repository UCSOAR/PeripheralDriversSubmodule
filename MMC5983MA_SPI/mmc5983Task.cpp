 /**
 ********************************************************************************
 * @file    MMC5983MATask.cpp
 * @author  Javier
 * @date    2026-01-10
 * @brief   Implementation of the MMC5983MA task handling.
 ********************************************************************************
 */

 /************************************
 * INCLUDES
 ************************************/
#include "mmc5983Task.hpp"
#include "main.h"
#include "DataBroker.hpp"


// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"

 /************************************
 * MACROS AND DEFINES
 ************************************/
/* temp definitions if not SystemDefines.hpp*/
#ifndef MMC_TASK_QUEUE_DEPTH
#define MMC_TASK_QUEUE_DEPTH 5
#endif

#ifndef MMC_TASK_STACK_DEPTH
#define MMC_TASK_STACK_DEPTH 256 // Adjust..
#endif

#ifndef MMC_TASK_PRIORITY
#define MMC_TASK_PRIORITY 3
#endif

 /************************************
 * FUNCTION DEFINITIONS
 ************************************/

 // Constructor
 MMC5983MATask::MMC5983MATask() : Task(MMC_TASK_QUEUE_DEPTH)
{
    _spi_wrapper = nullptr;
    _magnetometer = nullptr;

    _enableReading = true;
    _enableLogging = true;
}

void MMC5983MATask::Init(SPI_HandleTypeDef* hspi)
{
    SOAR_ASSERT(hspi != nullptr, "MMC5983MATask: Received Null SPI Handle");
    
    _spi_wrapper = new SPI_Wrapper(hspi);
    _magnetometer = new MMC5983MA(_spi_wrapper, MMC_CS_PORT, MMC_CS_PIN);
}


void MMC5983MATask::InitTask() // RTOS Task Init
{
    // Make sure dependencies are set
    SOAR_ASSERT(_magnetometer != nullptr, "MMC5983MATask: Driver not initialized. Call Init(hspi) first.");

    // Assert Task not already created
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize MMC5983MA task twice");
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)MMC5983MATask::RunTask,
            (const char*)"MMC5983MATask",
            (uint16_t)MMC_TASK_STACK_DEPTH,
            (void*)this,
            (UBaseType_t)MMC_TASK_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

     SOAR_ASSERT(rtValue == pdPASS, "MMC5983MATask::InitTask() - xTaskCreate() failed");
}

void MMC5983MATask::GetLatestData(MagData & dataOut)
{
    dataOut = _lastReading;
}

void MMC5983MATask::Run(void * pvParams)  // Instance Run loop for task
{
   /*   -Driver Setup-  */

   // Initialize 1x Gain
    if (_magnetometer->begin() != MMC5983MA_Status::OK){
        // Handle initialization error
        SOAR_PRINT("MMC5983MATask: Sensor initialization failed.\n");
        _enableReading = false;

    } 
    else{
        SOAR_PRINT("MMC5983MATask: Sensor initialized successfully.\n");
    }

    MagData magData;

    /* == Main Loop == */
    while (1)
    {
        // Check if reading is enabled
        if (_enableReading){

            _magnetometer->triggerMeasurement();
            vTaskDelay(pdMS_TO_TICKS(10)); // Wait for measurement to complete
            
            // Read sensor data
            if (_magnetometer->readData(magData) == MMC5983MA_Status::OK){
                
                _lastReading = magData;

                if (_enableLogging) {
                    SOAR_PRINT("MMC5983MATask: Magnetometer Reading: %ld, %ld, %ld\n", magData.rawX, magData.rawY, magData.rawZ);
                }
                // TODO: Send data somewhere
                DataBroker::Publish<MagData>(&magData);
                Command logCommand(DATA_BROKER_COMMAND, DataBrokerMessageTypes::MAG_DATA);
                LoggingTask::Inst().GetEventQueue()->Send(flashCommand);

            }
            else{

                if (_enableLogging) {
                    SOAR_PRINT("MMC5983MATask: Failed to read sensor data.\n");
                }
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

void MMC5983MATask::HandleCommand(Command & cm)
{
    switch (cm.GetTaskCommand())
    {
    // TODO: Add command cases (Gain change, calibration, etc.) IF NEEDED
    
    case MMC5983MA_Commands::MMC_CMD_START_READ: // Start Readings
        _enableReading = true;
        SOAR_PRINT("MMC5983MATask: Enabled Readings.\n");
        break;

    case MMC5983MA_Commands::MMC_CMD_STOP_READ: // Stop Readings
        _enableReading = false;
        SOAR_PRINT("MMC5983MATask: Disabled Readings.\n");
        break;

    case MMC5983MA_Commands::MMC_CMD_ENABLE_LOG: // Enable Logging
        _enableLogging = true;
        SOAR_PRINT("MMC5983MATask: Enabled Logging.\n");
        break;

    case MMC5983MA_Commands::MMC_CMD_DISABLE_LOG: // Disable Logging
        _enableLogging = false;
        SOAR_PRINT("MMC5983MATask: Disabled Logging.\n");
        break;

    default:
        SOAR_PRINT("MMC5983MATask: Received Unsupported Command {%d}.\n", cm.GetTaskCommand());
        break;
    }

    cm.Reset();
}
