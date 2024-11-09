/**
 ********************************************************************************
 * @file    Driver.hpp
 * @author  connell-reffo
 * @date    Nov 9, 2024
 * @brief
 ********************************************************************************
 */

#ifndef SOARDRIVERS_NAFE11388_DRIVER_INC_DRIVER_HPP_
#define SOARDRIVERS_NAFE11388_DRIVER_INC_DRIVER_HPP_

/************************************
 * INCLUDES
 ************************************/
#include <stm32h7xx_hal_gpio.h>
#include <stm32h7xx_hal_spi.h>
#include <stdbool.h>

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

namespace NAFE11388 {
	/************************************
	 * CLASS DEFINITIONS
	 ************************************/
	class Driver {
	private:
		SPI_HandleTypeDef *spi_handle;
		GPIO_TypeDef *gpio_port;
		uint16_t gpio_pin;
		uint32_t max_timeout;

	public:
		Driver(SPI_HandleTypeDef *hqspi, GPIO_TypeDef *gpio_port, uint16_t gpio_pin, uint32_t max_timeout = HAL_MAX_DELAY) :
			spi_handle(hqspi),
			gpio_port(gpio_port),
			gpio_pin(gpio_pin),
			max_timeout(max_timeout)
		{};

		bool write_register(uint8_t reg_addr, uint16_t value);
		bool read_register(uint8_t reg_addr, uint16_t &output);
	};
}

/************************************
 * FUNCTION DECLARATIONS
 ************************************/


#endif /* SOARDRIVERS_NAFE11388_DRIVER_INC_DRIVER_HPP_ */
