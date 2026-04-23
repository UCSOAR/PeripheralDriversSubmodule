/*
 * PollingTask.hpp
 *
 *  Created on: Apr 8, 2026
 *      Author: jaddina
 */
#ifndef COMPONENTS_POLLINGTASK_HPP_
#define COMPONENTS_POLLINGTASK_HPP_

#include "Task.hpp"
#include "MS5607Driver.hpp"
#include "MS5611Driver.hpp"
#include "lsm6dso.hpp"
#include <LSM6DO32Driver.h>
#include "mmc5983ma.hpp"
#include "NEO-M9N-00BDriver.hpp"

#include "SensorDataTypes.hpp"
#include "main.h"

class CanAutoNodeDaughter;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi4;
extern SPI_HandleTypeDef hspi5;
extern SPI_HandleTypeDef hspi6;

enum RocketState
{
    //-- GROUND --
    // Manual venting allowed at all times
    RS_PRELAUNCH = 0,   // Idle state, waiting for command to proceeding sequences
    RS_FILL,        // N2 Prefill/Purge/Leak-check/Load-cell Tare check sub-sequences, full control of valves (except MEV) allowed
    RS_ARM,         // We don't allow fill etc. 1-2 minutes before launch : Cannot fill rocket with N2 etc. unless you return to FILL
                    // Power Transition, Fill-Arm Disconnect Sub-sequences (you should be able to revert the power transition)

    //-- IGNITION -- Manual venting NOT ALLOWED
    RS_IGNITION,    // Ignition of the ignitors
    RS_LAUNCH,      // Launch triggered by confirmation of ignition (from ignitor) is nominal : MEV Open Sequence

    //-- BURN --
    // Vents should stay closed, manual venting NOT ALLOWED
    // !vents open is definitely not ideal for abort! Best to keep it closed with manual override if we fail here
    // (can we maybe have the code change the true default state by overwriting EEPROM?)
    // Ideally we don't want to EVER exceed 7 seconds of burn time (we should store this time at the very least - split into 1 sub-stage for each second if necessary).
    // For timing we want ~1/10th of a second or better.
    RS_BURN,        // Main burn (vents closed MEV open) - 5-6 seconds (TBD) :


    //-- COAST --
    // Manual venting NOT ALLOWED -- Note: MEV never closes!
    RS_COAST,       // Coasting (MEV closed, vents closed) - 30 seconds (TBD) ^ Vents closed applies here too, in part. Includes APOGEE

    //-- DESCENT / POSTAPOGEE --
    // Automatic Venting AND Vent Control ALLOWED
    RS_DESCENT,  // Vents open (well into the descent)
    RS_RECOVERY,  // Vents open, MEV closed, transmit all data over radio and accept vent commands
                  // Supports general commands (e.g. venting) and logs/transmits slowly (maybe stop logging after close to full memory?)

    //-- RECOVERY / TECHNICAL --
    RS_ABORT,       // Abort sequence, vents open, MEV closed, ignitors off
    RS_TEST,        // Test, between ABORT and PRE-LAUNCH, has full control of all GPIOs

    RS_NONE         // Invalid state, must be last
};




class PollingTask: public Task
{
	public:

		enum PollingTaskCommands : uint16_t {
			POLL_SENSORS_AND_LOG = 1,
			GPS_TEST,
		};

		static PollingTask& Inst() {
			static PollingTask inst;
			return inst;
		}

		void InitTask();
		void SetRocketState(RocketState newState);
		RocketState GetRocketState() const { return rocketState; }



	protected:

		static void RunTask(void* pvParams) { PollingTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
		void HandleRequestCommand(uint16_t taskCommand);
		static void PollTimerCallback(TimerHandle_t xTimer);



	private:
		// Private Functions
		PollingTask();        // Private constructor
		PollingTask(const PollingTask&);                        // Prevent copy-construction
		PollingTask& operator=(const PollingTask&);				// Prevent assignment
		void LogData();
		void PollSensors();
		void ApplyRocketState();
		void ServiceCanNetwork();
		static bool DecodeRocketStateFromCan(uint8_t rawState, RocketState& outState);
		static uint32_t GetPollingPeriodMs(RocketState state);
		TimerHandle_t pollTimerHandle;

		//to count time elapsed and time taken per poll and Hz
		TickType_t pollingStartTick = 0;
		TickType_t previousLogTick = 0;
		bool pollingTimerStarted = false;
		bool gpsInitialized = false;
		bool canNetworkReady = false;
		TickType_t lastCanJoinRetryTick = 0;

		RocketState rocketState;
		//sensor data structs
		BaroData baro07Data;
		BaroData baro11Data;
		IMUData imu32Data;
		IMUData imu16Data;
		MagData magData;
		MagDriverData driverData;
		GPSData gpsData;

		//All sensor gpios and spi
		//Baro 07 config
		GPIO_TypeDef* MS5607_CS_PORT = BARO07_CS_GPIO_Port;
		const uint16_t MS5607_CS_PIN = BARO07_CS_Pin;
		SPI_HandleTypeDef* hspi3_= &hspi3;

		//Baro 11 config
		GPIO_TypeDef* MS5611_CS_PORT = BARO11_CS_GPIO_Port;
		const uint16_t MS5611_CS_PIN = BARO11_CS_Pin;
		SPI_HandleTypeDef* hspi1_ = &hspi1;

		//IMU 32 config
		GPIO_TypeDef *LSM6DSO32_CS_PORT = IMU32_CS_GPIO_Port;
		const uint16_t LSM6DSO32_CS_PIN = IMU32_CS_Pin;
		SPI_HandleTypeDef *hspi2_ = &hspi2;

		//IMU 16 config
		GPIO_TypeDef *LSM6DSO_CS_PORT = IMU16_CS_GPIO_Port;
		const uint16_t LSM6DSO_CS_PIN = IMU16_CS_Pin;
		SPI_HandleTypeDef *hspi6_ = &hspi6;

		//Mag config
	    SPI_HandleTypeDef *hspi4_ = &hspi4;
	    GPIO_TypeDef *MMC_CS_PORT = MAG_CS_GPIO_Port;
	    const uint16_t MMC_CS_PIN = MAG_CS_Pin;

	    //GPS config
	    GPIO_TypeDef *GPS_CS_PORT = GPS_CS_GPIO_Port;
	   	const uint16_t GPS_CS_PIN = GPS_CS_Pin;


	    //Drivers
	    LSM6DSO_Driver imu16;
	    LSM6DO32_Driver imu32;
	    MS5607_Driver barometer07{hspi3_, MS5607_CS_PORT, MS5607_CS_PIN};
	    MS5611_Driver barometer11{hspi1_, MS5611_CS_PORT, MS5611_CS_PIN};
	    MMC5983MA magnetometer;
	    NEOM9N00B gps;

	    CanAutoNodeDaughter* canNode = nullptr;

};



#endif /* COMPONENTS_POLLINGTASK_HPP_ */
