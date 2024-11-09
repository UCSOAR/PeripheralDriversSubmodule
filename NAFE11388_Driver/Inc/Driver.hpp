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

		uint16_t channel_cfg_0;
		uint16_t channel_cfg_1;
		uint16_t channel_cfg_2;

	public:
		Driver(SPI_HandleTypeDef *hqspi, GPIO_TypeDef *gpio_port, uint16_t gpio_pin, uint32_t max_timeout = HAL_MAX_DELAY) :
			spi_handle(hqspi),
			gpio_port(gpio_port),
			gpio_pin(gpio_pin),
			max_timeout(max_timeout),
			channel_cfg_0(0),
			channel_cfg_1(0),
			channel_cfg_2(0)
		{};

		bool write_register(uint8_t reg_addr, uint16_t value) const;
		bool read_register(uint8_t reg_addr, uint16_t &output) const;

		void cfg_hv_aip(uint8_t bits_3);
		void cfg_hv_ain(uint8_t bits_3);
		void cfg_ch_gain(uint8_t bits_2);
		void cfg_hv_sel(uint8_t bits_1);
		void cfg_lvsig_in(uint8_t bits_2);
		void cfg_tcc_off(uint8_t bits_1);
		void cfg_ch_cal_gain_offset(uint8_t bits_3);
		void cfg_ch_thrs(uint8_t bits_3);
		void cfg_adc_data_rate(uint8_t bits_4);
		void cfg_adc_sinc(uint8_t bits_2);
		void cfg_ch_delay(uint8_t bits_5);
		void cfg_adc_normal_settling(uint8_t bits_1);
		void cfg_adc_filter_reset(uint8_t bits_1);

		bool write_cfg() const;
	};
}

/************************************
 * FUNCTION DECLARATIONS
 ************************************/


#endif /* SOARDRIVERS_NAFE11388_DRIVER_INC_DRIVER_HPP_ */
