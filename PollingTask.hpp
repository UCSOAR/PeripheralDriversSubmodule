/*
 * PollingTask.hpp
 *
 *  Created on: Apr 8, 2026
 *      Author: jaddina
 */
#ifndef COMPONENTS_POLLINGTASK_HPP_
#define COMPONENTS_POLLINGTASK_HPP_

#include "Task.hpp"
//#include "MS5607Driver.hpp"
//#include "MS5611Driver.hpp"
//#include "lsm6dso.hpp"
//#include <LSM6DO32Driver.h>
//#include "mmc5983ma.hpp"
//#include "NEO-M9N-00BDriver.hpp"

//#include "SensorDataTypes.hpp"
#include "main.h"


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

//		enum PollingTaskCommands : uint16_t {
//			POLL_SENSORS_AND_LOG = 1,
//			GPS_TEST,
//		};

		static PollingTask& Inst() {
			static PollingTask inst;
			return inst;
		}

		void InitTask();
//		void SetRocketState(RocketState newState);
//		RocketState GetRocketState() const { return rocketState; }

	protected:

		static void RunTask(void* pvParams) { PollingTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
		void HandleRequestCommand(uint16_t taskCommand);
//		static void PollTimerCallback(TimerHandle_t xTimer);

	private:
		// Private Functions
		PollingTask();        // Private constructor
		PollingTask(const PollingTask&);                        // Prevent copy-construction
		PollingTask& operator=(const PollingTask&);				// Prevent assignment
//		void LogData();
//		void PollSensors();
//		void ApplyRocketState();
//		void ServiceCanNetwork();
//		static bool DecodeRocketStateFromCan(uint8_t rawState, RocketState& outState);
//		static uint32_t GetPollingPeriodMs(RocketState state);
//		TimerHandle_t pollTimerHandle;
//
//		//to count time elapsed and time taken per poll and Hz
//		TickType_t pollingStartTick = 0;
//		TickType_t previousLogTick = 0;
//		bool pollingTimerStarted = false;
//		bool canNetworkReady = false;
//		TickType_t lastCanJoinRetryTick = 0;
//
//		RocketState rocketState;
//		//sensor data structs
//
//	    CanAutoNodeDaughter* canNode = nullptr;

};



#endif /* COMPONENTS_POLLINGTASK_HPP_ */
