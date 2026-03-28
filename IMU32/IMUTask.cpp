/*
 * IMUTask.cpp
 *
 *  Created on: Jan 27, 2026
 *      Author: jaddina
 */

/************************************
 * INCLUDES
 ************************************/
#include <IMU32/IMUTask.hpp>
#include "SystemDefines.hpp"
#include "Command.hpp"
#include "LoggingService.hpp"
#include "DataBroker.hpp"
#include "Task.hpp"
#include "LoggingTask.hpp"
/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
namespace
{
	int32_t AbsInt32(int32_t value)
	{
		return (value < 0) ? -value : value;
	}
}

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/
IMUTask::IMUTask() : Task(TASK_LOGGING_QUEUE_DEPTH_OBJS), imu()
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
					(const char *)"IMUTask",
					(uint16_t)TASK_LOGGING_QUEUE_DEPTH_WORDS,
					(void *)this,
					(UBaseType_t)TASK_LOGGING_PRIORITY,
					(TaskHandle_t *)&rtTaskHandle);

	SOAR_ASSERT(rtValue == pdPASS, "IMUTask::InitTask() - xTaskCreate() failed");
}

void IMUTask::Run(void *pvParams)
{

	imu.Init(hspi_, LSM6DSO32_CS_PORT, LSM6DSO32_CS_PIN);

	while (1)
	{
		imu.ReadSensors(data);
		imu_data = imu.ConvertRawMeasurementToStruct(data);
		imu_data.id = 1;

		const int32_t gx_mdps = static_cast<int32_t>(imu_data.gyro.x);
		const int32_t gy_mdps = static_cast<int32_t>(imu_data.gyro.y);
		const int32_t gz_mdps = static_cast<int32_t>(imu_data.gyro.z);
		const int32_t ax_mg = static_cast<int32_t>(imu_data.accel.x);
		const int32_t ay_mg = static_cast<int32_t>(imu_data.accel.y);
		const int32_t az_mg = static_cast<int32_t>(imu_data.accel.z);

		const char *gx_sign = (gx_mdps < 0) ? "-" : "";
		const char *gy_sign = (gy_mdps < 0) ? "-" : "";
		const char *gz_sign = (gz_mdps < 0) ? "-" : "";
		const char *ax_sign = (ax_mg < 0) ? "-" : "";
		const char *ay_sign = (ay_mg < 0) ? "-" : "";
		const char *az_sign = (az_mg < 0) ? "-" : "";

		const int32_t gx_abs_mdps = AbsInt32(gx_mdps);
		const int32_t gy_abs_mdps = AbsInt32(gy_mdps);
		const int32_t gz_abs_mdps = AbsInt32(gz_mdps);
		const int32_t ax_abs_mg = AbsInt32(ax_mg);
		const int32_t ay_abs_mg = AbsInt32(ay_mg);
		const int32_t az_abs_mg = AbsInt32(az_mg);

		SOAR_PRINT("IMU Data Accel(g)=[%s%d.%03d,%s%d.%03d,%s%d.%03d] Gyro(dps)=[%s%d.%03d,%s%d.%03d,%s%d.%03d] Temp(C)=%d\n",
				   ax_sign, static_cast<int>(ax_abs_mg / 1000), static_cast<int>(ax_abs_mg % 1000),
				   ay_sign, static_cast<int>(ay_abs_mg / 1000), static_cast<int>(ay_abs_mg % 1000),
				   az_sign, static_cast<int>(az_abs_mg / 1000), static_cast<int>(az_abs_mg % 1000),
				   gx_sign, static_cast<int>(gx_abs_mdps / 1000), static_cast<int>(gx_abs_mdps % 1000),
				   gy_sign, static_cast<int>(gy_abs_mdps / 1000), static_cast<int>(gy_abs_mdps % 1000),
				   gz_sign, static_cast<int>(gz_abs_mdps / 1000), static_cast<int>(gz_abs_mdps % 1000),
				   imu_data.temp);

		// LogData();

		Command cm;
		bool res = qEvtQueue->Receive(cm, 333);
		if (res)
		{

			HandleCommand(cm);
		}

		cm.Reset();
	}
}

void IMUTask::HandleCommand(Command &cm)
{
	switch (cm.GetCommand())
	{
	case DATA_COMMAND:
		HandleRequestCommand(cm.GetTaskCommand());
		break;

	case TASK_SPECIFIC_COMMAND:
		break;

	default:
		SOAR_PRINT("No valid global command given");
	}
}
void IMUTask::HandleRequestCommand(uint16_t taskCommand)
{
	switch (taskCommand)
	{
	case IMUTask::IMU_SAMPLE_AND_LOG:
	{
		imu.ReadSensors(data);
		imu_data = imu.ConvertRawMeasurementToStruct(data);
		imu_data.id = 1;
		LogData();
	}
	default:
		break;
	}
}

void IMUTask::LogData()
{

	DataBroker::Publish<IMUData>(&imu_data);
	Command logCommand(DATA_BROKER_COMMAND, static_cast<uint16_t>(DataBrokerMessageTypes::IMU_DATA)); // change if separate publisher
	osDelay(10);
	LoggingTask::Inst().GetEventQueue()->Send(logCommand);

	// SOAR_PRINT("Data Sent to LoggingTask\n");
}
