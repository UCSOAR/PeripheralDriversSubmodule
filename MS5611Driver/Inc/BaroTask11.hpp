/*
 * BaroTask11.hpp
 *
 *  Created on: Jan 26, 2026
 *      Author: jaddina
 */

#ifndef PERIPHERALDRIVERSSUBMODULE_MS5607DRIVER_INC_BAROTASK11_HPP_
#define PERIPHERALDRIVERSSUBMODULE_MS5607DRIVER_INC_BAROTASK11_HPP_

#include "Task.hpp"
#include "MS5611Driver.hpp"
#include "SensorDataTypes.hpp"
#include "main.h"
#include "main.c"
/************************************
 * MACROS AND DEFINES
 ************************************/


/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
enum BARO_TASK_COMMANDS{
	BARO11_SAMPLE_AND_LOG,
	NONE
};
/************************************
 * FUNCTION DECLARATIONS
 ************************************/
class BaroTask11: public Task
{
	public:
		static BaroTask11& Inst() {
			static BaroTask11 inst;
			return inst;
		}

		void InitTask();



	protected:
		static void RunTask(void* pvParams) { BaroTask11::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
		void HandleRequestCommand(uint16_t taskCommand);
		BaroData data;
		MS5611_Driver barometer;
		GPIO_TypeDef* MS5611_CS_PORT = BARO11_CS_GPIO_Port;
		const uint16_t MS5611_CS_PIN = BARO11_CS_Pin; //adjust when needed
		SPI_HandleTypeDef* hspi_ = &hspi2;// adjust this when needed





	private:
		// Private Functions
		BaroTask11();        // Private constructor
		BaroTask11(const BaroTask11&);                        // Prevent copy-construction
		BaroTask11& operator=(const BaroTask11&);														// Prevent assignment
		void LogData();
};








#endif /* PERIPHERALDRIVERSSUBMODULE_MS5607DRIVER_INC_BAROTASK11_HPP_ */
