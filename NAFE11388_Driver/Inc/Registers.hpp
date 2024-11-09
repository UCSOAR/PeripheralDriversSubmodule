/**
 ********************************************************************************
 * @file    NAFE11388_Registers.hpp
 * @author  connell-reffo
 * @date    Nov 9, 2024
 * @brief	https://www.nxp.com/docs/en/data-sheet/NAFE11388.pdf
 ********************************************************************************
 */

#ifndef SOARDRIVERS_NAFE11388_DRIVER_INC_NAFE11388_REGISTERS_HPP_
#define SOARDRIVERS_NAFE11388_DRIVER_INC_NAFE11388_REGISTERS_HPP_

/************************************
 * INCLUDES
 ************************************/
#include <stdint.h>

/**
 * See page 73 of the datasheet
 */
namespace NAFE11388::Registers {
	/************************************
	 * CONSTANTS
	 ************************************/
	namespace CMD_CH {
		constexpr uint8_t CMD_CH0 = 0x00;
		constexpr uint8_t CMD_CH1 = 0x01;
		constexpr uint8_t CMD_CH2 = 0x02;
		constexpr uint8_t CMD_CH3 = 0x03;
		constexpr uint8_t CMD_CH4 = 0x04;
		constexpr uint8_t CMD_CH5 = 0x05;
		constexpr uint8_t CMD_CH6 = 0x06;
		constexpr uint8_t CMD_CH7 = 0x07;
		constexpr uint8_t CMD_CH8 = 0x08;
		constexpr uint8_t CMD_CH9 = 0x09;
		constexpr uint8_t CMD_CH10 = 0x0A;
		constexpr uint8_t CMD_CH11 = 0x0B;
		constexpr uint8_t CMD_CH12 = 0x0C;
		constexpr uint8_t CMD_CH13 = 0x0D;
		constexpr uint8_t CMD_CH14 = 0x0E;
		constexpr uint8_t CMD_CH15 = 0x0F;
	}

	namespace CH_CONFIG {
		/**
		 * HV_AIP[3:0] = CH_CONFIG0[15:12]
		 * HV_AIN[3:0] = CH_CONFIG0[11:8]
		 * CH_GAIN[2:0] = CH_CONFIG0[7:5]
		 * HV_SEL = CH_CONFIG0[4]
		 * LVSIG_IN[2:0] = CH_CONFIG0[3:1]
		 * TCC_OFF = CH_CONFIG0[0]
		 */
		constexpr uint8_t CH_CONFIG0 = 0x20;

		/**
		 * CH_CAL_GAIN_OFFSET[3:0] = CH_CONFIG1[15:12]
		 * CH_THRS[3:0] = CH_CONFIG1[11:8]
		 * ADC_DATA_RATE[4:0] = CH_CONFIG1[7:3]
		 * ADC_SINC[2:0] = CH_CONFIG1[2:0]
		 */
		constexpr uint8_t CH_CONFIG1 = 0x21;

		/**
		 * CH_DELAY[5:0] = CH_CONFIG2[15:10]
		 * ADC_NORMAL_SETTLING = CH_CONFIG2[9]
		 * ADC_FILTER_RESET = CH_CONFIG2[8]
		 * CH_CHOP = CH_CONFIG2[7]
		 * CH_CONFIG2[6:0] (RESERVED)
		 */
		constexpr uint8_t CH_CONFIG2 = 0x22;
	}

	namespace GAIN_COEF {
		constexpr uint8_t GAIN_COEF0 = 0x80;
		constexpr uint8_t GAIN_COEF1 = 0x81;
		constexpr uint8_t GAIN_COEF2 = 0x82;
		constexpr uint8_t GAIN_COEF3 = 0x83;
		constexpr uint8_t GAIN_COEF4 = 0x84;
		constexpr uint8_t GAIN_COEF5 = 0x85;
		constexpr uint8_t GAIN_COEF6 = 0x86;
		constexpr uint8_t GAIN_COEF7 = 0x87;
		constexpr uint8_t GAIN_COEF8 = 0x88;
		constexpr uint8_t GAIN_COEF9 = 0x89;
		constexpr uint8_t GAIN_COEF10 = 0x8A;
		constexpr uint8_t GAIN_COEF11 = 0x8B;
		constexpr uint8_t GAIN_COEF12 = 0x8C;
		constexpr uint8_t GAIN_COEF13 = 0x8D;
		constexpr uint8_t GAIN_COEF14 = 0x8E;
		constexpr uint8_t GAIN_COEF15 = 0x8F;
	}

	/**
	 * Offset coefficient registers
	 */
	namespace OFFSET_COEF {
		constexpr uint8_t OFFSET_COEF0 = 0x90;
		constexpr uint8_t OFFSET_COEF1 = 0x91;
		constexpr uint8_t OFFSET_COEF2 = 0x92;
		constexpr uint8_t OFFSET_COEF3 = 0x93;
		constexpr uint8_t OFFSET_COEF4 = 0x94;
		constexpr uint8_t OFFSET_COEF5 = 0x95;
		constexpr uint8_t OFFSET_COEF6 = 0x96;
		constexpr uint8_t OFFSET_COEF7 = 0x97;
		constexpr uint8_t OFFSET_COEF8 = 0x98;
		constexpr uint8_t OFFSET_COEF9 = 0x99;
		constexpr uint8_t OFFSET_COEF10 = 0x9A;
		constexpr uint8_t OFFSET_COEF11 = 0x9B;
		constexpr uint8_t OFFSET_COEF12 = 0x9C;
		constexpr uint8_t OFFSET_COEF13 = 0x9D;
		constexpr uint8_t OFFSET_COEF14 = 0x9E;
		constexpr uint8_t OFFSET_COEF15 = 0x9F;
	}

	/**
	 * Optional calibration registers
	 */
	namespace OPT_COEF {
		constexpr uint8_t OPT_COEF0 = 0xA0;
		constexpr uint8_t OPT_COEF1 = 0xA1;
		constexpr uint8_t OPT_COEF2 = 0xA2;
		constexpr uint8_t OPT_COEF3 = 0xA3;
		constexpr uint8_t OPT_COEF4 = 0xA4;
		constexpr uint8_t OPT_COEF5 = 0xA5;
		constexpr uint8_t OPT_COEF6 = 0xA6;
		constexpr uint8_t OPT_COEF7 = 0xA7;
		constexpr uint8_t OPT_COEF8 = 0xA8;
		constexpr uint8_t OPT_COEF9 = 0xA9;
		constexpr uint8_t OPT_COEF10 = 0xAA;
		constexpr uint8_t OPT_COEF11 = 0xAB;
		constexpr uint8_t OPT_COEF12 = 0xAC;
		constexpr uint8_t OPT_COEF13 = 0xAD;
	}

	namespace CH_DATA {
		constexpr uint8_t CH_DATA0 = 0x40;
		constexpr uint8_t CH_DATA1 = 0x41;
		constexpr uint8_t CH_DATA2 = 0x42;
		constexpr uint8_t CH_DATA3 = 0x43;
		constexpr uint8_t CH_DATA4 = 0x44;
		constexpr uint8_t CH_DATA5 = 0x45;
		constexpr uint8_t CH_DATA6 = 0x46;
		constexpr uint8_t CH_DATA7 = 0x47;
		constexpr uint8_t CH_DATA8 = 0x48;
		constexpr uint8_t CH_DATA9 = 0x49;
		constexpr uint8_t CH_DATA10 = 0x4A;
		constexpr uint8_t CH_DATA11 = 0x4B;
		constexpr uint8_t CH_DATA12 = 0x4C;
		constexpr uint8_t CH_DATA13 = 0x4D;
		constexpr uint8_t CH_DATA14 = 0x4E;
		constexpr uint8_t CH_DATA15 = 0x4F;
	}

	namespace CH_STATUS {
		constexpr uint8_t CH_STATUS0 = 0x35;
		constexpr uint8_t CH_STATUS1 = 0x36;
	}
}

#endif /* SOARDRIVERS_NAFE11388_DRIVER_INC_NAFE11388_REGISTERS_HPP_ */
