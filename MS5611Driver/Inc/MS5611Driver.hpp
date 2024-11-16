/** ********************************************************************************
 * @file    MS5611Driver.hpp
 * @author  Noah
 * @date    Nov 9, 2024
 * @brief
 ******************************************************************************** */
#ifndef SOARDRIVERS_MS5611DRIVER_INC_MS5611DRIVER_HPP_
#define SOARDRIVERS_MS5611DRIVER_INC_MS5611DRIVER_HPP_
/************************************ * INCLUDES ************************************/
#include "SystemDefines.hpp"
#include "cmsis_os.h"
/************************************ * MACROS AND DEFINES ************************************/
/************************************ * TYPEDEFS ************************************/
typedef uint8_t MS5611_REGISTER_t;

struct MS5611_DATA_t{
	int16_t temp;
	uint32_t pressure;
};
/************************************ * CLASS DEFINITIONS ************************************/
class MS5611_Driver{
public:
	MS5611_Driver();
	void Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_);

	MS5611_DATA_t getSample();
private:
	// variables
	bool initialized = false;

	//constants
	GPIO_TypeDef* cs_gpio;
	uint16_t cs_pin;
	SPI_HandleTypeDef* hspi;

	bool SetRegister(MS5611_REGISTER_t reg, uint8_t val);
	uint16_t ReadCalibrationCoefficients(uint8_t PROM_READ_CMD);
};

/************************************ * FUNCTION DECLARATIONS ************************************/
#endif /* EXAMPLE_TASK_HPP_ */
