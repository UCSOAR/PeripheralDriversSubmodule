/*
 * MagTask.hpp
 *
 *  Created on: Jan 27, 2026
 *      Author: jaddina
 */

#ifndef PERIPHERALDRIVERSSUBMODULE_COMPONENTS_MAGTASK_HPP_
#define PERIPHERALDRIVERSSUBMODULE_COMPONENTS_MAGTASK_HPP_



#include "SensorDataTypes.hpp"
#include "Task.hpp"
#include "LIS3MDLTRDriver.h"
/************************************
 * MACROS AND DEFINES
 ************************************/


/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
enum MAG_TASK_COMMANDS{
	MAG_NONE,
	MAG_SAMPLE_AND_LOG,
};
/************************************
 * FUNCTION DECLARATIONS
 ************************************/
class MagTask: public Task
{
	public:
		static MagTask& Inst() {
			static MagTask inst;
			return inst;
		}

		void InitTask();



	protected:
		static void RunTask(void* pvParams) { MagTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
		void HandleRequestCommand(uint16_t taskCommand);
		MagData2 mag_data;
		uint8_t data[8];
		LIS3MDLTR_Driver magnetometer;
		GPIO_TypeDef* LIS3MDLTR_CS_PORT = GPIOA;
		const uint16_t LIS3MDLTR_CS_PIN = GPIO_PIN_4; //adjust when needed
		SPI_HandleTypeDef* hspi_ = 4;// adjust this when needed





	private:
		// Private Functions
		MagTask();        // Private constructor
		MagTask(const MagTask&);                        // Prevent copy-construction
		MagTask& operator=(const MagTask&);														// Prevent assignment
		void LogData();
};







#endif /* PERIPHERALDRIVERSSUBMODULE_COMPONENTS_MAGTASK_HPP_ */
