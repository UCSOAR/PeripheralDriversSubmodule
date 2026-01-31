/*
 * BaroTask07.hpp
 *
 *  Created on: Jan 26, 2026
 *      Author: jaddina
 */

#ifndef PERIPHERALDRIVERSSUBMODULE_MS5607DRIVER_INC_BAROTASK07_HPP_
#define PERIPHERALDRIVERSSUBMODULE_MS5607DRIVER_INC_BAROTASK07_HPP_
#include "Task.hpp"
#include "MS5607Driver.hpp"
#include "SensorDataTypes.hpp"
/************************************
 * MACROS AND DEFINES
 ************************************/


/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
enum BARO07_TASK_COMMANDS {
	BARO07_NONE,
	BARO07_SAMPLE_AND_LOG

};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/
class BaroTask07: public Task
{
	public:
		static BaroTask07& Inst() {
			static BaroTask07 inst;
			return inst;
		}

		void InitTask();



	protected:
		static void RunTask(void* pvParams) { BaroTask07::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
		void Run(void * pvParams); // Main run code
		void HandleCommand(Command& cm);
		void HandleRequestCommand(uint16_t taskCommand);
		BaroData data;
		MS5607Driver barometer;
		GPIO_TypeDef* MS5607_CS_PORT = GPIOA;
		const uint16_t MS5607_CS_PIN = GPIO_PIN_4; //adjust when needed
		SPI_HandleTypeDef* hspi_ = 4;// adjust this when needed





	private:
		// Private Functions
		BaroTask07();        // Private constructor
		BaroTask07(const BaroTask07&);                        // Prevent copy-construction
		BaroTask07& operator=(const BaroTask07&);														// Prevent assignment
		void LogData();
};

#endif /* PERIPHERALDRIVERSSUBMODULE_MS5607DRIVER_INC_BAROTASK07_HPP_ */
