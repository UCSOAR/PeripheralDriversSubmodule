/*
 * MagTask.cpp
 *
 *  Created on: Jan 27, 2026
 *      Author: jaddina
 */
/************************************
 * INCLUDES
 ************************************/
#include "MagTask.hpp"
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
MagTask::MagTask():Task(TASK_LOGGING_QUEUE_DEPTH_OBJS)
{

}

/**
 * @brief Initialize the MagTask
 *        Do not modify this function aside from adding the task name
 */
void MagTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)MagTask::RunTask,
            (const char*)"MagTask",
            (uint16_t)TASK_LOGGING_QUEUE_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_LOGGING_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "MagTask::InitTask() - xTaskCreate() failed");
}

void MagTask::Run(void * pvParams){

	magnetometer = LSM6DO32_Driver();
	magnetometer.Init(hspi, LSM6DO32_CS_PORT, LSM6DO32_CS_PIN);

    while (1) {
        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res){

        	HandleCommand(cm);
        }
    }
}

void MagTask::HandleCommand(Command& cm){
	switch(cm.getCommand()){
	case DATA_COMMAND:
		HandleRequestCommand(cm.GetTaskCommand());
		break;

	case TASK_SPECIFIC_COMMAND:
		break;
	}



}
void MagTask::HandleRequestCommand(uint16_t taskCommand){
	switch(taskCommand){
	case MAG_SAMPLE_AND_LOG:
		magnetometer.GetMeasurements(data);
		mag_data = magnetometer.GetDataFromBuf(data);
		LogData();
	default:
		break;
	}


}

void MagTask::LogData(){
	DataBroker::Publish<MagData2>(&data);
	Command logCommand(DATA_BROKER_COMMAND, DataBrokerMessageTypes::MAG_DATA2); //change if separate publisher
	LoggingTask::Inst().GetEventQueue()->Send(flashCommand);

}

