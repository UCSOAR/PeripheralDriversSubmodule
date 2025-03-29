/**
 ********************************************************************************
 * @file    LIS3DH_Driver.cpp
 * @author  Christy
 * @date    Mar 1, 2025
 * @brief	gets x/y/z acceleration data from LIS3DH accelerometer
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "LIS3DH_Driver.hpp"
/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/
LIS3DH_DRIVER::LIS3DH_DRIVER(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_):
	hspi(hspi_), cs_gpio(cs_gpio_), cs_pin(cs_pin_){

	write_register(0x20, 0x00); // set CTRL_REG1 to 0 for normal mode
	write_register(0x23, 0x00); // set CTRL_REG4 to 0 for normal mode
};

void LIS3DH_DRIVER::write_register(uint8_t reg, uint8_t data){
	uint8_t datap[2] = {reg, data};
	HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, datap, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}

LIS3DH_DATA LIS3DH_DRIVER::get_acceleration(){
	uint8_t dataBuffer[6];
	LIS3DH_DATA accelerationData;

	//set cs pin low; spi mode
	HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(hspi, (uint8_t[]){0x28 | 0x80}, dataBuffer, 6, 100);

	accelerationData.x = (uint16_t)((dataBuffer[1] << 8) | dataBuffer[0]);
	accelerationData.y = (uint16_t)((dataBuffer[3] << 8) | dataBuffer[2]);
	accelerationData.z = (uint16_t)((dataBuffer[5] << 8) | dataBuffer[4]);

	//set cs pin high; end spi transmission
	HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);

	//return accelerationData;
	return convert_acceleration(accelerationData);
};

LIS3DH_DATA LIS3DH_DRIVER::convert_acceleration(LIS3DH_DATA values){
	LIS3DH_DATA converted_data;

	// sensitivity for normal mode = 4
	uint8_t coefficient = 4*981/1000;		// divide by 100 to get acceleration in m/s^2

	// convert values from 2's complement mg
	converted_data.x = values.x*coefficient;
	converted_data.y = values.y*coefficient;
	converted_data.z = values.z*coefficient;

	return converted_data;
}


