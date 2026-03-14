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
#include "LoggingTask.hpp"
#include "lsm6dso.hpp"
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
LSM6DSOTask::LSM6DSOTask() : Task(TASK_LOGGING_QUEUE_DEPTH_OBJS), imu()
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
					(const char *)"LSM6DSOTask",
					(uint16_t)TASK_LOGGING_QUEUE_DEPTH_WORDS,
					(void *)this,
					(UBaseType_t)TASK_LOGGING_PRIORITY,
					(TaskHandle_t *)&rtTaskHandle);

	SOAR_ASSERT(rtValue == pdPASS, "LSM6DSOTask::InitTask() - xTaskCreate() failed");
}

void LSM6DSOTask::Run(void *pvParams)
{

	imu.Init(hspi_, LSM6DSO_CS_PIN, LSM6DSO_CS_PORT);

	while (1)
	{
		HAL_Delay(500);
		imu.readSensors(data);
		imu_data = imu.bytesToStruct(data, true, true, true);
		imu_data.id = 0;

		SOAR_PRINT("IMU Data Accel(mg)=[%d,%d,%d] Gyro(mdps)=[%d,%d,%d] Temp(C)=%d\n",
				   imu_data.accel.x, imu_data.accel.y, imu_data.accel.z,
				   imu_data.gyro.x, imu_data.gyro.y, imu_data.gyro.z,
				   imu_data.temp);

		LogData();
		Command cm;
		bool res = qEvtQueue->Receive(cm, 333);
		if (res)
		{

			HandleCommand(cm);
		}
	}
}

void LSM6DSOTask::HandleCommand(Command &cm)
{
	switch (cm.GetCommand())
	{
	case DATA_COMMAND:
		HandleRequestCommand(cm.GetTaskCommand());
		break;
	case DATA_BROKER_COMMAND:
		SOAR_PRINT("Not data command");
		break;

	case TASK_SPECIFIC_COMMAND:
		break;

	case COMMAND_NONE:
		SOAR_PRINT("No command");
		break;
	}
	cm.Reset();
}
void LSM6DSOTask::HandleRequestCommand(uint16_t taskCommand)
{
	switch (taskCommand)
	{
	case LSM6DSOTask::IMU_SAMPLE_AND_LOG:
	{

		LogData();
	}
	default:
		break;
	}
}

void LSM6DSOTask::LogData()
{

	DataBroker::Publish<IMUData>(&imu_data);
	//	Command logCommand(DATA_BROKER_COMMAND, static_cast<uint16_t>(DataBrokerMessageTypes::IMU_DATA)); //change if separate publisher
	//	LoggingTask::Inst().GetEventQueue()->Send(logCommand);

	// SOAR_PRINT("Data Sent to LoggingTask\n");
}
