#include "IRCTramp.hpp"
#include "stm32g4xx_ll_usart.h"

bool IRCTramp::SetVTXFrequency(FREQUENCY freq, USART_TypeDef* uart) {

	uint8_t tx_buffer[16];
	uint8_t checksum = 0;

	for(uint16_t i = 0; i < sizeof(tx_buffer); i++) {
		tx_buffer[i] = 0;
	}

	uint16_t freqMHz;
	switch(freq) {
	case FREQ_1258MHZ:
		freqMHz = 1258;
		break;
	case FREQ_1280MHZ:
		freqMHz = 1280;
		break;
	default:
		return false;
	}

	tx_buffer[0] = 0x0F;                   // start byte
	tx_buffer[1] = 'v';                    // v = frequency
	tx_buffer[2] = freqMHz & 0xFF;        // freq LSB
	tx_buffer[3] = (freqMHz >> 8) & 0xFF; // freq MSB

	for (uint8_t i = 1; i < sizeof(tx_buffer)-1; i++) {
		checksum += tx_buffer[i];
	}
	tx_buffer[15] = checksum;

	for(uint8_t i = 0; i < sizeof(tx_buffer); i++) {
		LL_USART_TransmitData8(uart, tx_buffer[i]);
		while (!LL_USART_IsActiveFlag_TXE(uart));
	}

	return true;
}

bool IRCTramp::SetVTXPower(POWER power, USART_TypeDef* uart) {
	uint8_t tx_buffer[16];
	uint8_t checksum = 0;

	for(uint16_t i = 0; i < sizeof(tx_buffer); i++) {
		tx_buffer[i] = 0;
	}

	uint16_t powerMW;
	switch(power) {
	case POWER_PIT:
		powerMW = 0;
		break;
	case POWER_25MW:
		powerMW = 25;
		break;
	case POWER_200MW:
		powerMW = 200;
		break;
	case POWER_1W:
		powerMW = 1000;
		break;
	case POWER_4W:
		powerMW = 4000;
		break;
	default:
		return false;
	}

	tx_buffer[0] = 0x0F;                   // start byte
	tx_buffer[1] = 'p';                    // p = power
	tx_buffer[2] = powerMW & 0xFF;        // power LSB
	tx_buffer[3] = (powerMW >> 8) & 0xFF; // power MSB

	for (uint8_t i = 1; i < sizeof(tx_buffer)-1; i++) {
		checksum += tx_buffer[i];
	}
	tx_buffer[15] = checksum;

	for(uint8_t i = 0; i < sizeof(tx_buffer); i++) {
		LL_USART_TransmitData8(uart, tx_buffer[i]);
		while (!LL_USART_IsActiveFlag_TXE(uart));
	}
	return true;
}

