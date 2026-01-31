/*
 * LSM6DSOTask.cpp
 *
 *  Created on: Jan 30, 2026
 *      Author: jaddina
 */
/************************************
 * INCLUDES
 ************************************/
#include "LSM6DSOTask.hpp"
#include "SystemDefines.hpp"
#include "Command.hpp"
#include "LoggingService.hpp"
#include "DataBroker.hpp"
#include "Task.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/



/************************************
 * FUNCTION DEFINITIONS
 ************************************/
LSM6DSOTask::LSM6DSOTask():Task(TASK_LOGGING_QUEUE_DEPTH_OBJS)
{

}

/**
 * @brief Initialize the LSM6DSOTask
 *        Do not modify this function aside from adding the task name
 */
void LSM6DSOTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)LSM6DSOTask::RunTask,
            (const char*)"LSM6DSOTask",
            (uint16_t)TASK_LOGGING_QUEUE_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_LOGGING_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "LSM6DSOTask::InitTask() - xTaskCreate() failed");
}

void LSM6DSOTask::Run(void * pvParams){

	LSM6DSO_Driver imu = LSM6DSO_Driver();
	imu.Init(hspi, LSM6DSO_CS_PIN, LSM6DSO_CS_PORT);

    while (1) {
        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res){

        	HandleCommand(cm);
        }
    }
}

void LSM6DSOTask::HandleCommand(Command& cm){
	switch(cm.getCommand()){
	case DATA_COMMAND:
		HandleRequestCommand(cm.GetTaskCommand());
		break;

	case TASK_SPECIFIC_COMMAND:
		break;
	}



}
void LSM6DSOTask::HandleRequestCommand(uint16_t taskCommand){
	switch(taskCommand){
	case IMU_SAMPLE_AND_LOG:
		imu.readSensors(data);
		imu_data = imu.bytesToStruct(data, true, true, true);
		imu_data.id = 0;
		LogData();
	default:
		break;
	}


}

void LSM6DSOTask::LogData(){
	DataBroker::Publish<IMUData>(&imu_data);
	Command logCommand(DATA_BROKER_COMMAND, DataBrokerMessageTypes::IMU_DATA); //change if separate publisher
	LoggingTask::Inst().GetEventQueue()->Send(logCommand);

}
