/**
 ********************************************************************************
 * @file    Bits.cpp
 * @author  connell-reffo
 * @date    Nov 9, 2024
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
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
namespace NAFE11388::Bits {
	void set_bit(uint32_t &value, BitPosition pos) {
		value |= 1 << static_cast<uint8_t>(pos);
	}

	void clear_bit(uint32_t &value, BitPosition pos) {
		value &= ~(1 << static_cast<uint8_t>(pos));
	}
}
