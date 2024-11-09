/**
 ********************************************************************************
 * @file    Bits.hpp
 * @author  connell-reffo
 * @date    Nov 9, 2024
 * @brief
 ********************************************************************************
 */

#ifndef SOARDRIVERS_NAFE11388_DRIVER_INC_BITS_HPP_
#define SOARDRIVERS_NAFE11388_DRIVER_INC_BITS_HPP_

/************************************
 * INCLUDES
 ************************************/
#include <stdint.h>

namespace NAFE11388::Bits {
	/************************************
	 * MACROS AND DEFINES
	 ************************************/

	/************************************
	 * TYPEDEFS
	 ************************************/

	/************************************
	 * CLASS DEFINITIONS
	 ************************************/
	enum class BitPosition : uint8_t {
		RA0 = 0,
		RA1 = 1,
		RA2 = 2,
		RA3 = 3,
		RA4 = 4,
		RA5 = 5,
		RA6 = 6,
		RA7 = 7,
		RA8 = 8,
		RA9 = 9,
		RA10 = 10,
		RA11 = 11,
		RA12 = 12,
		RW_L = 13
	};

	/************************************
	 * FUNCTION DECLARATIONS
	 ************************************/
	void set_bit(uint32_t &value, BitPosition pos);
	void clear_bit(uint32_t &value, BitPosition pos);
}

#endif /* SOARDRIVERS_NAFE11388_DRIVER_INC_BITS_HPP_ */
