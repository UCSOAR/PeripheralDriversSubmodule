/*
 * file: lsm6dso.cpp
 *
 *  Created on: Jan 29, 2026
 *      Author: Jad Dina
 */


/* Includes ------------------------------------------------------------------*/
#include "lsm6dso.hpp"

LSM6DSO_Driver::LSM6DSO_Driver(){

}

void LSM6DSO_Driver::Init(SPI_HandleTypeDef* hspi_, uint8_t cs_pin_, GPIO_TypeDef* cs_gpio_){
	hspi = hspi_;
	cs_pin = cs_pin_;
	cs_gpio = cs_gpio_;
	CS_High();

	//TODO read from WHO_AM_I reg to confirm initialization


}

void LSM6DSO_Driver::setRegister(LSM6DSO_REGISTER_t reg, uint8_t val){

	uint8_t tx[2] = {0b00000000 | (0x7f & reg), val};// write MSB must be 0 ensures MSB is 0
	//transmit spi message
	CS_Low();
	HAL_StatusTypeDef result = HAL_SPI_Transmit(hspi, tx, 2, 1000);
	CS_High();

}

uint8_t LSM6DSO_Driver::getRegister(LSM6DSO_REGISTER_t reg){
	uint8_t tx[2] = {0b10000000 | (0x7f & reg), 0x00};  //read MSB must be 1 ensures MSB is 1
	uint8_t rx[2]= {0,0};
	//transmit address of reg and recieve reg data spi message
	CS_Low();
	HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(hspi, tx, rx, 2, 1000);
	CS_High();
	return rx[1];
}




LSM6DSO_Driver::CS_High(){

	HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}


LSM6DSO_Driver::CS_High(){

	HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}
