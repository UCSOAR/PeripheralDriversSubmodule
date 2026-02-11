/*
 * IMUTask.cpp
 *
 *  Created on: Jan 27, 2026
 *      Author: jaddina
 */

/************************************
 * INCLUDES
 ************************************/
#include "IMUTask.hpp"
#include "SystemDefines.hpp"
#include "Command.hpp"
#include "LoggingService.hpp"
#include "DataBroker.hpp"
#include "Task.hpp"
#include "LoggingTask.hpp"
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
IMUTask::IMUTask():Task(TASK_LOGGING_QUEUE_DEPTH_OBJS), imu()
{

}

/**
 * @brief Initialize the IMUTask
 *        Do not modify this function aside from adding the task name
 */
void IMUTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)IMUTask::RunTask,
            (const char*)"IMUTask",
            (uint16_t)TASK_LOGGING_QUEUE_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_LOGGING_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "IMUTask::InitTask() - xTaskCreate() failed");
}

void IMUTask::Run(void * pvParams){


	imu.Init(hspi_, LSM6DSO32_CS_PORT, LSM6DSO32_CS_PIN);

    while (1) {
        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res){

        	HandleCommand(cm);
        }
    }
}

void IMUTask::HandleCommand(Command& cm){
	switch(cm.GetCommand()){
	case DATA_COMMAND:
		HandleRequestCommand(cm.GetTaskCommand());
		break;

	case TASK_SPECIFIC_COMMAND:
		break;

	default:
			SOAR_PRINT("No valid global command given");
	}




}
void IMUTask::HandleRequestCommand(uint16_t taskCommand){
	switch(taskCommand){
	case IMUTask::IMU_SAMPLE_AND_LOG:
		while(1){
			imu.ReadSensors(data);
			imu_data = imu.ConvertRawMeasurementToStruct(data);
			imu_data.id = 1;
			osDelay(1000);
			LogData();
		}
	default:
		break;
	}


}

void IMUTask::LogData(){
	//Print driver output
	SOAR_PRINT("IMU32 accelX: %d\n", imu_data.accel.x);
	SOAR_PRINT("IMU32 accelY: %d\n", imu_data.accel.y);
	SOAR_PRINT("IMU32 accelZ: %d\n", imu_data.accel.z);

	SOAR_PRINT("IMU32 gyroX: %d\n", imu_data.gyro.x);
	SOAR_PRINT("IMU32 gyroY: %d\n", imu_data.gyro.y);
	SOAR_PRINT("IMU32 gyroZ: %d\n", imu_data.gyro.z);

	SOAR_PRINT("IMU32 temperature: %d\n", imu_data.temp);

	DataBroker::Publish<IMUData>(&imu_data);
	Command logCommand(DATA_BROKER_COMMAND, static_cast<uint16_t>(DataBrokerMessageTypes::IMU_DATA)); //change if separate publisher
	LoggingTask::Inst().GetEventQueue()->Send(logCommand);

	SOAR_PRINT("Data Sent to LoggingTask\n");


}

