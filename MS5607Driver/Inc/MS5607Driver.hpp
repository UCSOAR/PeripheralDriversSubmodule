/** ********************************************************************************
 * @file    MS5607Driver.hpp
 * @author  Noah
 * @date    Nov 16, 2024
 * @brief	This driver gets pressure and temperature readings from the MS5607 barometers.
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
/**
 * @brief the driver for MS5607 barometers
 * see datasheet here: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5607-02BA03%7FB%7Fpdf%7FEnglish%7FENG_DS_MS5607-02BA03_B.pdf%7FCAT-BLPS0035
 */
class MS5607_Driver{
public:
	MS5607_Driver(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_):
		hspi(hspi_), cs_gpio(cs_gpio_), cs_pin(cs_pin_){}

	MS5607_DATA_t getSample();
private:

	//constants
	GPIO_TypeDef* cs_gpio;
	uint16_t cs_pin;
	SPI_HandleTypeDef* hspi;

	bool SetRegister(MS5607_REGISTER_t reg, uint8_t val);
	uint16_t ReadCalibrationCoefficients(uint8_t PROM_READ_CMD);
};

/************************************ * FUNCTION DECLARATIONS ************************************/
#endif /* EXAMPLE_TASK_HPP_ */
