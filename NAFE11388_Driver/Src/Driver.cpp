/**
 ********************************************************************************
 * @file    Driver.cpp
 * @author  connell-reffo
 * @date    Nov 9, 2024
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "../Inc/Driver.hpp"
#include "../Inc/Bits.hpp"

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
namespace NAFE11388 {
	bool Driver::write_register(uint8_t reg_addr, uint16_t value) const {
		HAL_GPIO_WritePin(this->gpio_port, this->gpio_pin, GPIO_PIN_RESET);

		if (!HAL_SPI_Transmit(this->spi_handle, &reg_addr, 1, this->max_delay)) {
			return false;
		}

		if (!HAL_SPI_Transmit(this->spi_handle, &value, 2, this->max_delay)) {
			return false;
		}

		HAL_GPIO_WritePin(this->gpio_port, this->gpio_pin, GPIO_PIN_SET);

		return true;
	}

	bool Driver::read_register(uint8_t reg_addr, uint16_t &output) const {
		using namespace Bits;

		set_bit(reg_addr, BitPositions::RW_L);

		HAL_GPIO_WritePin(this->gpio_port, this->gpio_pin, GPIO_PIN_RESET);

		if (!HAL_SPI_Transmit(this->spi_handle, &reg_addr, 1, this->max_delay)) {
			return false;
		}

		if (!HAL_SPI_Receive(this->spi_handle, &output, 2, this->max_delay)) {
			return false;
		}

		HAL_GPIO_WritePin(this->gpio_port, this->gpio_pin, GPIO_PIN_SET);

		return true;
	}
}
