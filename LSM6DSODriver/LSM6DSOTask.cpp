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
namespace
{
	constexpr uint16_t LSM6DSO_GYRO_CALIBRATION_SAMPLES = 500;
	constexpr uint32_t LSM6DSO_GYRO_CALIBRATION_DELAY_MS = 5;

	int16_t ClampInt16(int32_t value)
	{
		if (value > 32767)
		{
			return 32767;
		}
		if (value < -32768)
		{
			return -32768;
		}
		return static_cast<int16_t>(value);
	}

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
	CalibrateGyroBias();

	while (1)
	{
		HAL_Delay(500);
		imu.readSensors(data);
		imu_data = imu.bytesToStruct(data, true, true, true);
		imu_data.id = 0;
		ApplyGyroBias();

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

		LogData();
		Command cm;
		bool res = qEvtQueue->Receive(cm, 333);
		if (res)
		{

			HandleCommand(cm);
		}
	}
}

void LSM6DSOTask::CalibrateGyroBias()
{
	int32_t sumX = 0;
	int32_t sumY = 0;
	int32_t sumZ = 0;

	SOAR_PRINT("Starting gyro zero-rate calibration, keep board still...\n");

	for (uint16_t sample = 0; sample < LSM6DSO_GYRO_CALIBRATION_SAMPLES; sample++)
	{
		HAL_Delay(LSM6DSO_GYRO_CALIBRATION_DELAY_MS);
		imu.readSensors(data);
		IMUData sampleData = imu.bytesToStruct(data, false, true, true);

		sumX += sampleData.gyro.x;
		sumY += sampleData.gyro.y;
		sumZ += sampleData.gyro.z;
	}

	gyro_bias.x = static_cast<int16_t>(sumX / LSM6DSO_GYRO_CALIBRATION_SAMPLES);
	gyro_bias.y = static_cast<int16_t>(sumY / LSM6DSO_GYRO_CALIBRATION_SAMPLES);
	gyro_bias.z = static_cast<int16_t>(sumZ / LSM6DSO_GYRO_CALIBRATION_SAMPLES);

	const int32_t bx_mdps = static_cast<int32_t>(gyro_bias.x);
	const int32_t by_mdps = static_cast<int32_t>(gyro_bias.y);
	const int32_t bz_mdps = static_cast<int32_t>(gyro_bias.z);

	const char *bx_sign = (bx_mdps < 0) ? "-" : "";
	const char *by_sign = (by_mdps < 0) ? "-" : "";
	const char *bz_sign = (bz_mdps < 0) ? "-" : "";

	const int32_t bx_abs_mdps = AbsInt32(bx_mdps);
	const int32_t by_abs_mdps = AbsInt32(by_mdps);
	const int32_t bz_abs_mdps = AbsInt32(bz_mdps);

	SOAR_PRINT("Gyro bias(dps)=[%s%d.%03d,%s%d.%03d,%s%d.%03d]\n",
			   bx_sign, static_cast<int>(bx_abs_mdps / 1000), static_cast<int>(bx_abs_mdps % 1000),
			   by_sign, static_cast<int>(by_abs_mdps / 1000), static_cast<int>(by_abs_mdps % 1000),
			   bz_sign, static_cast<int>(bz_abs_mdps / 1000), static_cast<int>(bz_abs_mdps % 1000));
}

void LSM6DSOTask::ApplyGyroBias()
{
	imu_data.gyro.x = ClampInt16(static_cast<int32_t>(imu_data.gyro.x) - gyro_bias.x);
	imu_data.gyro.y = ClampInt16(static_cast<int32_t>(imu_data.gyro.y) - gyro_bias.y);
	imu_data.gyro.z = ClampInt16(static_cast<int32_t>(imu_data.gyro.z) - gyro_bias.z);
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
