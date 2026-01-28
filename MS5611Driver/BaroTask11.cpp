/*
 * BaroTask11.cpp
 *
 *  Created on: Jan 26, 2026
 *      Author: jaddina
 */



/************************************
 * INCLUDES
 ************************************/
#include "BaroTask11.hpp"
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
BaroTask11::BaroTask11():Task(TASK_LOGGING_QUEUE_DEPTH_OBJS)
{

}

/**
 * @brief Initialize the BaroTask11
 *        Do not modify this function aside from adding the task name
 */
void BaroTask11::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)BaroTask11::RunTask,
            (const char*)"BaroTask11",
            (uint16_t)TASK_LOGGING_QUEUE_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_LOGGING_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "BaroTask11::InitTask() - xTaskCreate() failed");
}

void BaroTask11::Run(void * pvParams){

	barometer = MS5611Driver(hspi, MS5611_CS_PORT, MS5611_CS_PIN);

    while (1) {
        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res){

        	HandleCommand(cm);
        }
    }
}

void BaroTask11::HandleCommand(Command& cm){
	switch(cm.getCommand()){
	case DATA_COMMAND:
		HandleRequestCommand(cm.GetTaskCommand());
		break;

	case TASK_SPECIFIC_COMMAND:
		break;
	}



}
void BaroTask11::HandleRequestCommand(uint16_t taskCommand){
	switch(taskCommand){
	case BARO11_SAMPLE_AND_LOG:
		data = barometer.getSample();
		LogData();
	default:
		break;
	}


}

void BaroTask11::LogData(){
	DataBroker::Publish<Baro11Data>(&data);
	Command logCommand(DATA_BROKER_COMMAND, DataBrokerMessageTypes::BARO11_DATA); //change if separate publisher
	LoggingTask::Inst().GetEventQueue()->Send(flashCommand);

}

