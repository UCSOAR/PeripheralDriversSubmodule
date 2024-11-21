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
namespace NAFE11388::Driver {
	bool write_reg(uint8_t reg_addr, uint8_t *value, uint8_t bytes) const {
		HAL_GPIO_WritePin(this->cs_gpio_port, this->cs_gpio_pin, GPIO_PIN_RESET);

		if (!HAL_SPI_Transmit(this->spi_handle, &reg_addr, 1, this->max_delay)) {
			return false;
		}

		if (!HAL_SPI_Transmit(this->spi_handle, value, bytes, this->max_delay)) {
			return false;
		}

		HAL_GPIO_WritePin(this->cs_gpio_port, this->cs_gpio_pin, GPIO_PIN_SET);

		return true;
	}

	bool read_reg(uint8_t reg_addr, uint8_t *output, uint8_t bytes) const {
		using namespace NAFE11388::Bits;

		constexpr uint8_t RW_L = 13;
		reg_addr |= 1 << RW_L;

		HAL_GPIO_WritePin(this->cs_gpio_port, this->cs_gpio_pin, GPIO_PIN_RESET);

		if (!HAL_SPI_Transmit(this->spi_handle, &reg_addr, 1, this->max_delay)) {
			return false;
		}

		if (!HAL_SPI_Receive(this->spi_handle, output, bytes, this->max_delay)) {
			return false;
		}

		HAL_GPIO_WritePin(this->cs_gpio_port, this->cs_gpio_pin, GPIO_PIN_SET);

		return true;
	}

	bool write_cfg() const {
		using namespace NAFE11388::Registers;

		return this->write_register(CH_CONFIG0, this->channel_cfg_0)
				&& this->write_register(CH_CONFIG1, this->channel_cfg_1)
				&& this->write_register(CH_CONFIG2, this->channel_cfg_2);
	}

//	bool wait_for_drdy(uint64_t countdown) const {
//		for (uint64_t i = 0; i < countdown; i--) {
//			if (HAL_GPIO_ReadPin(this->drdy_gpio_port, this->drdy_gpio_pin) == GPIO_PIN_SET) {
//				return true;
//			}
//		}
//
//		return false;
//	}

	void cfg_hv_aip(uint8_t bits_3) {
		bits_3 &= 0b111;

		// Zero out bits 15:12
		this->channel_cfg_0 &= ~(0b111 << 12);

		// Set bits 15:12
		this->channel_cfg_0 |= bits_3 << 12;
	}

	void cfg_hv_ain(uint8_t bits_3) {
		bits_3 &= 0b111;

		this->channel_cfg_0 &= ~(0b111 << 8);
		this->channel_cfg_0 |= bits_3 << 8;
	}

	void cfg_ch_gain(uint8_t bits_2) {
		bits_2 &= 0b11;

		this->channel_cfg_0 &= ~(0b11 << 5);
		this->channel_cfg_0 |= bits_2 << 5;
	}

	void cfg_hv_sel(uint8_t bits_1) {
		bits_1 &= 1;

		this->channel_cfg_0 &= ~(1 << 4);
		this->channel_cfg_0 |= bits_1 << 4;
	}

	void cfg_lvsig_in(uint8_t bits_2) {
		bits_2 &= 0b11;

		this->channel_cfg_0 &= ~(0b11 << 1);
		this->channel_cfg_0 |= bits_2 << 1;
	}

	void cfg_tcc_off(uint8_t bits_1) {
		bits_1 &= 1;

		this->channel_cfg_0 &= ~1;
		this->channel_cfg_0 |= bits_1;
	}

	void cfg_ch_cal_gain_offset(uint8_t bits_3) {
		bits_3 &= 0b111;

		this->channel_cfg_1 &= ~(0b111 << 12);
		this->channel_cfg_1 |= bits_3 << 12;
	}

	void cfg_ch_thrs(uint8_t bits_3) {
		bits_3 &= 0b111;

		this->channel_cfg_1 &= ~(0b111 << 8);
		this->channel_cfg_1 |= bits_3 << 8;
	}

	void cfg_adc_data_rate(uint8_t bits_4) {
		bits_4 &= 0b1111;

		this->channel_cfg_1 &= ~(0b1111 << 3);
		this->channel_cfg_1 |= bits_4 << 3;
	}

	void cfg_adc_sinc(uint8_t bits_2) {
		bits_2 &= 0b11;

		this->channel_cfg_1 &= ~0b11;
		this->channel_cfg_1 |= bits_2;
	}

	void cfg_ch_delay(uint8_t bits_5) {
		bits_5 &= 0b11111;

		this->channel_cfg_2 &= ~(0b11111 << 10);
		this->channel_cfg_2 |= bits_5 << 10;
	}

	void cfg_adc_normal_settling(uint8_t bits_1) {
		bits_1 &= 1;

		this->channel_cfg_2 &= ~(1 << 9);
		this->channel_cfg_2 |= bits_1 << 9;
	}

	void cfg_adc_filter_reset(uint8_t bits_1) {
		bits_1 &= 1;

		this->channel_cfg_2 &= ~(1 << 8);
		this->channel_cfg_2 |= bits_1 << 8;
	}
}
