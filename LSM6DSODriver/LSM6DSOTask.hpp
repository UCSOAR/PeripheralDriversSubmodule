/*
 * LSM6DSODriver.hpp
 *
 *  Created on: Jan 30, 2026
 *      Author: jaddina
 */

#ifndef PERIPHERALDRIVERSSUBMODULE_LSM6DSODRIVER_LSM6DSODRIVER_HPP_
#define PERIPHERALDRIVERSSUBMODULE_LSM6DSODRIVER_LSM6DSODRIVER_HPP_

#include "SensorDataTypes.hpp"
#include "Task.hpp"
#include "lsm6dso.hpp"
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
class LSM6DSOTask: public Task
{
	public:
		static LSM6DSOTask& Inst() {
			static LSM6DSOTask inst;
			return inst;
		}

		void InitTask();



	protected:
		static void RunTask(void* pvParams) { LSM6DSOTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
		void HandleRequestCommand(uint16_t taskCommand);
		IMUData imu_data;
		uint8_t data[14];
		GPIO_TypeDef* LSM6DSO_CS_PORT = GPIOA;
		const uint16_t LSM6DSO_CS_PIN = GPIO_PIN_4; //adjust when needed
		SPI_HandleTypeDef* hspi_ = 4;// adjust this when needed





	private:
		// Private Functions
		LSM6DSOTask();        // Private constructor
		LSM6DSOTask(const LSM6DSOTask&);                        // Prevent copy-construction
		LSM6DSOTask& operator=(const LSM6DSOTask&);														// Prevent assignment
		void LogData();
};







#endif /* PERIPHERALDRIVERSSUBMODULE_LSM6DSODRIVER_LSM6DSODRIVER_HPP_ */
