/**
 ********************************************************************************
 * @file    LIS3DH_Driver.hpp
 * @author  Christy
 * @date    Mar 1, 2025
 * @brief
 ********************************************************************************
 */

#ifndef SOARDRIVERS_INC_LIS3DH_DRIVER_HPP_
#define SOARDRIVERS_INC_LIS3DH_DRIVER_HPP_

/************************************
 * INCLUDES
 ************************************/

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/
struct LIS3DH_DATA {
	int16_t x;
	int16_t y;
	int16_t z;
};
/************************************
 * CLASS DEFINITIONS
 ************************************/
class LIS3DH_DRIVER{
public:
	LIS3DH_DRIVER(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_):
		hspi(hspi_), cs_gpio(cs_gpio_), cs_pin(cs_pin_){
	};

	LIS3DH_DATA get_acceleration();

private:
	GPIO_TypeDef* cs_gpio;
	uint16_t cs_pin;
	SPI_HandleTypeDef* hspi;

	void write_register(uint8_t reg, uint8_t data);
	LIS3DH_DATA convert_acceleration(LIS3DH_DATA values);
};
/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* SOARDRIVERS_INC_LIS3DH_DRIVER_HPP_ */
