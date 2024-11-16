/** ********************************************************************************
 * @file    MS5607Driver.hpp
 * @author  Noah
 * @date    Nov 16, 2024
 * @brief
 ******************************************************************************** */
#ifndef SOARDRIVERS_MS5607DRIVER_INC_MS5607DRIVER_HPP_
#define SOARDRIVERS_MS5607DRIVER_INC_MS5607DRIVER_HPP_
/************************************ * INCLUDES ************************************/
#include "SystemDefines.hpp"
#include "cmsis_os.h"
/************************************ * MACROS AND DEFINES ************************************/
/************************************ * TYPEDEFS ************************************/
typedef uint8_t MS5607_REGISTER_t;

struct MS5607_DATA_t{
	int16_t temp;
	uint32_t pressure;
};
/************************************ * CLASS DEFINITIONS ************************************/
class MS5607_Driver{
public:
	MS5607_Driver();
	void Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_);

	MS5607_DATA_t getSample();
private:
	// variables
	bool initialized = false;

	//constants
	GPIO_TypeDef* cs_gpio;
	uint16_t cs_pin;
	SPI_HandleTypeDef* hspi;

	bool SetRegister(MS5607_REGISTER_t reg, uint8_t val);
	uint16_t ReadCalibrationCoefficients(uint8_t PROM_READ_CMD);
};

/************************************ * FUNCTION DECLARATIONS ************************************/
#endif /* EXAMPLE_TASK_HPP_ */
