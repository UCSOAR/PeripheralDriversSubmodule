#include "RuncamUART.hpp"


uint8_t RuncamUARTDriver::Runcam_CRC8(uint8_t *data, uint8_t len) {
	uint8_t crc = 0x00;
	for (uint8_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (uint8_t j = 0; j < 8; j++) {
			if (crc & 0x80) {
				crc = (crc << 1) ^ 0xD5;
			} else {
				crc <<= 1;
			}
		}
	}
	return crc;
}

HAL_StatusTypeDef RuncamUARTDriver::Runcam_SendPacket(uint8_t command, uint8_t action) {

	if (huart == NULL) return HAL_ERROR;

	uint8_t packet[4];
	packet[0] = 0xCC;
	packet[1] = command;
	packet[2] = action;
	packet[3] = Runcam_CRC8(&packet[1], 2);

	return HAL_UART_Transmit(huart, packet, 4, 100);
}


HAL_StatusTypeDef RuncamUARTDriver::Runcam_SetRecording(RUNCAM_REC action) {
	return Runcam_SendPacket(0x03, (uint8_t)action);
}

HAL_StatusTypeDef RuncamUARTDriver::Runcam_SimulateButton(RUNCAM_BUTTON button) {
	return Runcam_SendPacket(0x01, (uint8_t)button);
}
