/*
 * PollingTask.cpp
 *
 *  Created on: Apr 8, 2026
 *      Author: jaddina
 */

#include "PollingTask.hpp"
#include "FreeRTOS.h"
#include "timers.h"
#include "DataBroker.hpp"
#include "LoggingService.hpp"
//#include "CanAutoNodeDaughter.hpp"

extern FDCAN_HandleTypeDef hfdcan1;

namespace {
constexpr uint32_t kLaunchPollMs = 20;
constexpr uint32_t kBoostPollMs = 50;
constexpr uint32_t kCoastPollMs = 100;
constexpr uint32_t kDescentPollMs = 100;
constexpr uint32_t kRecoveryPollMs = 250;
constexpr uint32_t kGroundPollMs = 300;
constexpr uint32_t kCanServicePeriodMs = 20;


constexpr uint8_t kDaqBoardType = 1;
constexpr uint8_t kDaqSlotNumber = 1;
constexpr uint8_t kRocketStateRxLogIndex = 0;
constexpr uint16_t kRocketStateRxLogSizeBytes = sizeof(uint8_t);
}

PollingTask::PollingTask():Task(TASK_LOGGING_QUEUE_DEPTH_OBJS), pollTimerHandle(nullptr), rocketState(RocketState::RS_TEST), imu16(), imu32(), magnetometer()
{

}

/**
 * @brief Initialize the PollingTask
 *        Do not modify this function aside from adding the task name
 */
void PollingTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)PollingTask::RunTask,
            (const char*)"PollingTask",
            (uint16_t)TASK_LOGGING_QUEUE_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_LOGGING_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "PollingTask::InitTask() - xTaskCreate() failed");

	pollTimerHandle = xTimerCreate("PollingTimer",
			pdMS_TO_TICKS(kLaunchPollMs),
			pdFALSE,
			(void*)this,
			PollingTask::PollTimerCallback);

	SOAR_ASSERT(pollTimerHandle != nullptr, "PollingTask::InitTask() - xTimerCreate() failed");
}

void PollingTask::SetRocketState(RocketState newState)
{
	rocketState = newState;
	ApplyRocketState();
}

uint32_t PollingTask::GetPollingPeriodMs(RocketState state)
{
	switch (state)
	{

	case RocketState::RS_NONE:
	case RocketState::RS_ABORT:
		LoggingService::StopLogging();
		return 0;
	case RocketState::RS_FILL:
		LoggingService::StartLogging();
		return kGroundPollMs;
	case RocketState::RS_PRELAUNCH:
		LoggingService::StartLogging();
		return kGroundPollMs;
	case RocketState::RS_ARM:
		LoggingService::StartLogging();
		return kGroundPollMs;
	case RocketState::RS_TEST:
		LoggingService::StopLogging();
		return 0;
	case RocketState::RS_IGNITION:
		LoggingService::StartLogging();
		return kLaunchPollMs;
	case RocketState::RS_LAUNCH:
		LoggingService::StartLogging();
		return kLaunchPollMs;
	case RocketState::RS_BURN:
		LoggingService::StartLogging();
		return kLaunchPollMs;
	case RocketState::RS_COAST:
		LoggingService::StartLogging();
		return kCoastPollMs;
	case RocketState::RS_DESCENT:
		LoggingService::StartLogging();
		return kDescentPollMs;
	case RocketState::RS_RECOVERY:
		LoggingService::StartLogging();
		return kRecoveryPollMs;

	default:
		return 0;
	}
}

void PollingTask::ApplyRocketState()
{
	if (pollTimerHandle == nullptr)
	{
		return;
	}

	const uint32_t pollPeriodMs = GetPollingPeriodMs(rocketState);
	if (pollPeriodMs == 0)
	{
		(void)xTimerStop(pollTimerHandle, 0);
		return;
	}

	(void)xTimerChangePeriod(pollTimerHandle, pdMS_TO_TICKS(pollPeriodMs), 0);
}

void PollingTask::Run(void * pvParams){
	// Allow peripherals to settle after scheduler starts, then initialize GPS.
	vTaskDelay(pdMS_TO_TICKS(kGroundPollMs));
	gpsInitialized = gps.Init(hspi6_, GPS_CS_PORT, GPS_CS_PIN);

    while (1) {

        Command cm;
		bool res = qEvtQueue->Receive(cm, kCanServicePeriodMs);
		if(res){

        	HandleCommand(cm);
        }

    }
}

bool PollingTask::DecodeRocketStateFromCan(uint8_t rawState, RocketState& outState)
{
	switch (rawState)
	{
	case static_cast<uint8_t>(RocketState::RS_PRELAUNCH):
		outState = RocketState::RS_PRELAUNCH;
		return true;
	case static_cast<uint8_t>(RocketState::RS_FILL):
		outState = RocketState::RS_FILL;
		return true;
	case static_cast<uint8_t>(RocketState::RS_ARM):
		outState = RocketState::RS_ARM;
		return true;
	case static_cast<uint8_t>(RocketState::RS_IGNITION):
		outState = RocketState::RS_IGNITION;
		return true;
	case static_cast<uint8_t>(RocketState::RS_LAUNCH):
		outState = RocketState::RS_LAUNCH;
		return true;
	case static_cast<uint8_t>(RocketState::RS_BURN):
		outState = RocketState::RS_BURN;
		return true;
	case static_cast<uint8_t>(RocketState::RS_COAST):
		outState = RocketState::RS_COAST;
		return true;
	case static_cast<uint8_t>(RocketState::RS_DESCENT):
		outState = RocketState::RS_DESCENT;
		return true;
	case static_cast<uint8_t>(RocketState::RS_RECOVERY):
		outState = RocketState::RS_RECOVERY;
		return true;
	case static_cast<uint8_t>(RocketState::RS_ABORT):
		outState = RocketState::RS_ABORT;
		return true;
	case static_cast<uint8_t>(RocketState::RS_TEST):
		outState = RocketState::RS_TEST;
		return true;
	case static_cast<uint8_t>(RocketState::RS_NONE):
		outState = RocketState::RS_NONE;
		return true;
	default:
		return false;
	}
}

void PollingTask::HandleCommand(Command& cm){
	switch(cm.GetCommand()){
	case DATA_COMMAND:
		HandleRequestCommand(cm.GetTaskCommand());
		break;

	case TASK_SPECIFIC_COMMAND:
		break;
	}

	cm.Reset();

}

void PollingTask::HandleRequestCommand(uint16_t taskCommand){
	switch(taskCommand){
	default:
		break;
	}
}

void PollingTask::PollTimerCallback(TimerHandle_t xTimer)
{
	PollingTask* task = static_cast<PollingTask*>(pvTimerGetTimerID(xTimer));
	if ((task == nullptr) || (task->GetEventQueue() == nullptr)) {
		return;
	}

	Command cmd(DATA_COMMAND, PollingTask::POLL_SENSORS_AND_LOG);
	task->GetEventQueue()->Send(cmd, true);
	task->ApplyRocketState();
}




void PollingTask::LogData(){
	TickType_t nowTick = xTaskGetTickCount();

	if(!pollingTimerStarted){
		pollingStartTick = nowTick;
		previousLogTick = nowTick;
		pollingTimerStarted = true;
	}

	TickType_t elapsedTicks = nowTick - pollingStartTick;
	TickType_t deltaTicks   = nowTick - previousLogTick;

	float elapsedSec = (float)elapsedTicks / configTICK_RATE_HZ;
	float deltaSec   = (float)deltaTicks / configTICK_RATE_HZ;
	uint32_t rateHz     = (deltaSec > 0.0f) ? (1.0f / deltaSec) : 0.0f;

	uint32_t elapsedMs = (elapsedTicks * 1000U) / configTICK_RATE_HZ;
	uint32_t deltaMs   = (deltaTicks * 1000U) / configTICK_RATE_HZ;

	SOAR_PRINT("PollingTask Timing | elapsed=%d ms dt=%d ms rate=%d Hz\n",
			   elapsedMs,
			   deltaMs,
			   rateHz);

	previousLogTick = nowTick;

	SOAR_PRINT("Log data polled");
//	SOAR_PRINT("PollingTask Log | Baro07 id=%u temp=%d pressure=%lu\n",
//			(unsigned int)baro07Data.id,
//			(int)baro07Data.temp,
//			(unsigned long)baro07Data.pressure);
//	SOAR_PRINT("PollingTask Log | Baro11 id=%u temp=%d pressure=%lu\n",
//			(unsigned int)baro11Data.id,
//			(int)baro11Data.temp,
//			(unsigned long)baro11Data.pressure);
//
//	SOAR_PRINT("PollingTask Log | IMU16 id=%u accel=(%d,%d,%d) gyro=(%d,%d,%d) temp=%d\n",
//			(unsigned int)imu16Data.id,
//			(int)imu16Data.accel.x,
//			(int)imu16Data.accel.y,
//			(int)imu16Data.accel.z,
//			(int)imu16Data.gyro.x,
//			(int)imu16Data.gyro.y,
//			(int)imu16Data.gyro.z,
//			(int)imu16Data.temp);
//
//	SOAR_PRINT("PollingTask Log | IMU32 id=%u accel=(%d,%d,%d) gyro=(%d,%d,%d) temp=%d\n",
//			(unsigned int)imu32Data.id,
//			(int)imu32Data.accel.x,
//			(int)imu32Data.accel.y,
//			(int)imu32Data.accel.z,
//			(int)imu32Data.gyro.x,
//			(int)imu32Data.gyro.y,
//			(int)imu32Data.gyro.z,
//			(int)imu32Data.temp);
//
//	SOAR_PRINT("PollingTask Log | MAG scaled=(%ld,%ld,%ld)\n",
//			(long)magData.magX,
//			(long)magData.magY,
//			(long)magData.magZ);

	DataBroker::Publish<IMUData>(&imu16Data);
	DataBroker::Publish<IMUData>(&imu32Data);
	DataBroker::Publish<BaroData>(&baro07Data);
	DataBroker::Publish<BaroData>(&baro11Data);
	DataBroker::Publish<MagData>(&magData);
	DataBroker::Publish<GPSData>(&gpsData);
}
