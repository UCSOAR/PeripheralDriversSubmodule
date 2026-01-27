/**
 ********************************************************************************
 * @file    MMC5983MATask.hpp
 * @author  Javier
 * @brief   FreeRTOS Task Wrapper for the MMC5983MA Magnetometer SPI Driver
 ********************************************************************************
 */

#ifndef MMC5983MA_TASK_HPP_
#define MMC5983MA_TASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "main.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// system includes
#include "SystemDefines.hpp"
#include "Task.hpp"
#include "SensorDataTypes.hpp"

// Driver Includes
#include "mmc5983ma.hpp"
#include "spi_wrapper.hpp"

/************************************
 * CLASS DEFINITIONS
 ************************************/
class MMC5983MATask : public Task
{
public:
    static MMC5983MATask& Inst(){
        static MMC5983MATask inst;
        return inst;
    }
    
    // Command Definitions:
    enum MMC5983MA_Commands {
        MMC_CMD_START_READ   = 1, // Start reading data
        MMC_CMD_STOP_READ    = 2, // Stop reading data
        MMC_CMD_DISABLE_LOG  = 3, // Disable logging
        MMC_CMD_ENABLE_LOG   = 4  // Enable logging
    };



    void Init(SPI_HandleTypeDef* hspi);

    void InitTask();

    void GetLatestData(MagData1& dataOut);

protected:
    static void RunTask(void* pvParams) { MMC5983MATask::Inst().Run(pvParams); }
    void Run(void* pvParams);
    void HandleCommand(Command& cm);

    MagData1 _lastReading;


private:
    // Constructors
    MMC5983MATask();
    MMC5983MATask(const MMC5983MATask&);
    MMC5983MATask& operator=(const MMC5983MATask&);

    // Obj to allow delayed inits
    SPI_Wrapper* _spi_wrapper;
    MMC5983MA* _magnetometer;

    // Task Control Flags
    bool _enableReading;
    bool _enableLogging;

    // Chip Select Port
    GPIO_TypeDef* MMC_CS_PORT = GPIOA;
    const uint16_t MMC_CS_PIN = GPIO_PIN_4; // Adjust as needed



};

/*
 * TODO: FINISH
*/

#endif /* MMC5983MA_TASK_HPP_ */
