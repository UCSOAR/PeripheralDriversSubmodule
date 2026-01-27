/*
 * IMUTask.hpp
 *
 *  Created on: Jan 27, 2026
 *      Author: jaddina
 */

#ifndef PERIPHERALDRIVERSSUBMODULE_COMPONENTS_IMUTASK_HPP_
#define PERIPHERALDRIVERSSUBMODULE_COMPONENTS_IMUTASK_HPP_



#include "SensorDataTypes.hpp"
#include "Task.hpp"
#include "LSM6DO32Driver.h"
/************************************
 * MACROS AND DEFINES
 ************************************/


/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
enum IMU_TASK_COMMANDS{
	IMU_NONE,
	IMU_SAMPLE_AND_LOG,
};
/************************************
 * FUNCTION DECLARATIONS
 ************************************/
class IMUTask: public Task
{
	public:
		static IMUTask& Inst() {
			static IMUTask inst;
			return inst;
		}

		void InitTask();



	protected:
		static void RunTask(void* pvParams) { IMUTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
		void HandleRequestCommand(uint16_t taskCommand);
		IMUData imu_data;
		uint8_t data[14];
		MS5611Driver barometer;
		GPIO_TypeDef* LSM6DO32_CS_PORT = GPIOA;
		const uint16_t LSM6DO32_CS_PIN = GPIO_PIN_4; //adjust when needed
		SPI_HandleTypeDef* hspi_ = 4;// adjust this when needed





	private:
		// Private Functions
		IMUTask();        // Private constructor
		IMUTask(const IMUTask&);                        // Prevent copy-construction
		IMUTask& operator=(const IMUTask&);														// Prevent assignment
		void LogData();
};


#endif /* PERIPHERALDRIVERSSUBMODULE_COMPONENTS_IMUTASK_HPP_ */
