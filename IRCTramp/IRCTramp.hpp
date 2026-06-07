/*
 * IRCTramp.hpp
 *
 *  Created on: Jun 6, 2026
 *      Author: Local user
 */

#ifndef IRCTRAMP_IRCTRAMP_HPP_
#define IRCTRAMP_IRCTRAMP_HPP_
#include <cstdint>

#ifdef STM32G491xx
#include "stm32g4xx.h"
#else
#include "stm32h7xx.h"
#endif

namespace IRCTramp {
enum FREQUENCY {
	FREQ_1258MHZ,
	FREQ_1280MHZ
};

enum POWER {
	POWER_PIT,
	POWER_25MW,
	POWER_200MW,
	POWER_1W,
	POWER_4W
};

bool SetVTXFrequency(FREQUENCY freq, UART_HandleTypeDef* uart);
bool SetVTXPower(POWER power, UART_HandleTypeDef* uart);

}



#endif /* IRCTRAMP_IRCTRAMP_HPP_ */
