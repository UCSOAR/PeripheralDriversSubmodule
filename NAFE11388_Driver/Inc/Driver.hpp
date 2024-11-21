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
#include "../Inc/Registers.hpp"

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

		GPIO_TypeDef *cs_gpio_port;
		uint16_t cs_gpio_pin;

		GPIO_TypeDef *drdy_gpio_port;
		uint16_t drdy_gpio_pin;

		uint32_t max_timeout;

		uint16_t channel_cfg_0;
		uint16_t channel_cfg_1;
		uint16_t channel_cfg_2;

		bool write_reg(uint8_t reg_addr, uint8_t *value, uint8_t bytes) const;
		bool read_reg(uint8_t reg_addr, uint8_t *output, uint8_t bytes) const;

		bool read_channel(constexpr uint8_t ch_addr, uint8_t *output) const {
			return this->read_reg_24(ch_addr, output);
		}

	public:
		Driver(SPI_HandleTypeDef *hqspi, GPIO_TypeDef *cs_gpio_port, uint16_t cs_gpio_pin, GPIO_TypeDef *drdy_gpio_port, uint16_t drdy_gpio_pin, uint32_t max_timeout = HAL_MAX_DELAY) :
			spi_handle(hqspi),
			cs_gpio_port(cs_gpio_port),
			cs_gpio_pin(cs_gpio_pin),
			drdy_gpio_port(drdy_gpio_port),
			drdy_gpio_pin(drdy_gpio_pin),
			max_timeout(max_timeout),
			channel_cfg_0(0),
			channel_cfg_1(0),
			channel_cfg_2(0)
		{};

		bool write_reg_24(uint8_t reg_addr, uint8_t *value) const {
			return this->write_reg(reg_addr, value, 3);
		}

		bool write_reg_16(uint8_t reg_addr, uint8_t *value) const {
			return this->write_reg(reg_addr, value, 2);
		}

		bool read_reg_24(uint8_t reg_addr, uint8_t *output) const {
			return this->read_reg(reg_addr, output, 3);
		}

		bool read_reg_16(uint8_t reg_addr, uint8_t *output) const {
			return this->read_reg(reg_addr, output, 2);
		}

		bool read_channel_0(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA0, output);
		}

		bool read_channel_1(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA1, output);
		}

		bool read_channel_2(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA2, output);
		}

		bool read_channel_3(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA3, output);
		}

		bool read_channel_4(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA4, output);
		}

		bool read_channel_5(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA5, output);
		}

		bool read_channel_6(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA6, output);
		}

		bool read_channel_7(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA7, output);
		}

		bool read_channel_8(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA8, output);
		}

		bool read_channel_9(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA9, output);
		}

		bool read_channel_10(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA10, output);
		}

		bool read_channel_11(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA11, output);
		}

		bool read_channel_12(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA12, output);
		}

		bool read_channel_13(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA13, output);
		}

		bool read_channel_14(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA14, output);
		}

		bool read_channel_15(uint8_t *output) {
			return this->read_channel(NAFE11388::Registers::CH_DATA::CH_DATA15, output);
		}

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

		// bool wait_for_drdy(uint64_t countdown = 0xFFFFFFFFFFFFFFFF) const;
	};
}

/************************************
 * FUNCTION DECLARATIONS
 ************************************/


#endif /* SOARDRIVERS_NAFE11388_DRIVER_INC_DRIVER_HPP_ */
