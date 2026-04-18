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

namespace {
constexpr uint32_t kLaunchPollMs = 20;
constexpr uint32_t kBoostPollMs = 20;
constexpr uint32_t kCoastPollMs = 100;
constexpr uint32_t kDescentPollMs = 100;
constexpr uint32_t kRecoveryPollMs = 250;
constexpr uint32_t kGpsBootSettleMs = 300;
}

PollingTask::PollingTask():Task(TASK_LOGGING_QUEUE_DEPTH_OBJS), pollTimerHandle(nullptr), flightState(FlightState::Grounded), imu16(), imu32(), magnetometer()
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

    //Init drivers
	imu16.Init(hspi6_, LSM6DSO_CS_PIN,  LSM6DSO_CS_PORT );
	imu32.Init(hspi2_, LSM6DSO32_CS_PORT, LSM6DSO32_CS_PIN);
	magnetometer.Init(hspi4_, MMC_CS_PORT, MMC_CS_PIN);

	pollTimerHandle = xTimerCreate("PollingTimer",
			pdMS_TO_TICKS(kLaunchPollMs),
			pdFALSE,
			(void*)this,
			PollingTask::PollTimerCallback);
	SOAR_ASSERT(pollTimerHandle != nullptr, "PollingTask::InitTask() - xTimerCreate() failed");

	ApplyFlightState();
}

void PollingTask::SetFlightState(FlightState newState)
{
	flightState = newState;
	ApplyFlightState();
}

uint32_t PollingTask::GetPollingPeriodMs(FlightState state)
{
	switch (state)
	{
	case FlightState::Grounded:
	case FlightState::Landed:
		return 0;
	case FlightState::Launch:
		return kLaunchPollMs;
	case FlightState::Boost:
		return kBoostPollMs;
	case FlightState::Coast:
		return kCoastPollMs;
	case FlightState::Descent:
		return kDescentPollMs;
	case FlightState::Recovery:
		return kRecoveryPollMs;
	default:
		return 0;
	}
}

void PollingTask::ApplyFlightState()
{
	if (pollTimerHandle == nullptr)
	{
		return;
	}

	const uint32_t pollPeriodMs = GetPollingPeriodMs(flightState);
	if (pollPeriodMs == 0)
	{
		(void)xTimerStop(pollTimerHandle, 0);
		return;
	}

	(void)xTimerChangePeriod(pollTimerHandle, pdMS_TO_TICKS(pollPeriodMs), 0);
}

void PollingTask::Run(void * pvParams){
	// Allow peripherals to settle after scheduler starts, then initialize GPS.
	vTaskDelay(pdMS_TO_TICKS(kGpsBootSettleMs));
	gpsInitialized = gps.Init(hspi6_, GPS_CS_PORT, GPS_CS_PIN);

    while (1) {
        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res){

        	HandleCommand(cm);
        }
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
	case POLL_SENSORS_AND_LOG:
		PollSensors();
		LogData();
		break;
	case GPS_TEST:
		if (gpsInitialized)
		{

			while(1){
				if (gps.getGGALine(gpsData.buffer_))
				{
					SOAR_PRINT("GPS RAW | %s\n", gpsData.buffer_);
					gps.ParseGpsData(&gpsData);
					SOAR_PRINT("GPS GGA | time=%d lat_deg=%d lat_min=%d lon_deg=%d lon_min=%d antAlt=%d antUnit=%d geoidAlt=%d geoidUnit=%d totalAlt=%d totalUnit=%d\n",
							   (int32_t)gpsData.time_,
							   (int32_t)gpsData.latitude_.degrees_,
							   (int32_t)gpsData.latitude_.minutes_,
							   (int32_t)gpsData.longitude_.degrees_,
							   (int32_t)gpsData.longitude_.minutes_,
							   (int32_t)gpsData.antennaAltitude_.altitude_,
							   (int32_t)gpsData.antennaAltitude_.unit_,
							   (int32_t)gpsData.geoidAltitude_.altitude_,
							   (int32_t)gpsData.geoidAltitude_.unit_,
							   (int32_t)gpsData.totalAltitude_.altitude_,
							   (int32_t)gpsData.totalAltitude_.unit_);
				}
			}
		}

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
	task->GetEventQueue()->Send(cmd, false);
	task->ApplyFlightState();
}


void PollingTask::PollSensors()
{
	uint8_t data[14] = {0};

	baro07Data = barometer07.getSample();
	baro07Data.id = 0;

	baro11Data = barometer11.getSample();
	baro11Data.id = 1;

	imu16.readSensors(data);
	imu16Data = imu16.bytesToStruct(data, true, true, true);
	imu16Data.id = 0;

	imu32.ReadSensors(data);
	imu32Data = imu32.ConvertRawMeasurementToStruct(data);
	imu32Data.id = 1;

	magnetometer.triggerMeasurement();
	magnetometer.readData(driverData);

	magData.magX = driverData.scaledX;
	magData.magY = driverData.scaledY;
	magData.magZ = driverData.scaledZ;
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



}
