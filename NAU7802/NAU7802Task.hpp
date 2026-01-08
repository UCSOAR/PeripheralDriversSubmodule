/**
 ********************************************************************************
 * @file    NAU7802Task.hpp
 * @author  Javier
 * @brief   FreeRTOS Task Wrapper for the NAU7802 ADC Driver
 ********************************************************************************
 */

#ifndef NAU7802_TASK_HPP_
#define NAU7802_TASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "Task.hpp"
#include "SystemDefines.hpp"
#include "NAU7802.hpp"

#include "i2c_wrapper.hpp"
#include "main.h"

/************************************
 * CLASS DEFINITIONS
 ************************************/
class NAU7802Task : public Task
{
public:
    static NAU7802Task& Inst(){
        static NAU7802Task inst;
        return inst;
    }

    // call in main passing &hi2c1 before InitTask
    void Init(I2C_HandleTypeDef* hi2c);

    void InitTask();

protected:
    static void RunTask(void* pvParams) { NAU7802Task::Inst().Run(pvParams); }
    void Run(void* pvParams);
    void HandleCommand(Command& cm);

private:
    // Constructors
    NAU7802Task();
    NAU7802Task(const NAU7802Task&);
    NAU7802Task& operator=(const NAU7802Task&);

    // Obj to allow delayed inits
    I2C_Wrapper* _i2c_wrapper;
    NAU7802* _adc;
};

#endif /* NAU7802_TASK_HPP_ */
