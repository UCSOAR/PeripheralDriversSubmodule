#include "stm32g4xx.h"


class RuncamUARTDriver {
public:
	enum RUNCAM_BUTTON {
		RUNCAM_BTN_WIFI  = 0x00,
		RUNCAM_BTN_POWER = 0x01, // Often used for Start/Stop Recording
		RUNCAM_BTN_MODE  = 0x02
	};

	enum RUNCAM_REC {
		RUNCAM_REC_START = 0x01,
		RUNCAM_REC_STOP  = 0x00
	};

	RuncamUARTDriver(UART_HandleTypeDef* huart): huart(huart){

	}

	HAL_StatusTypeDef Runcam_SetRecording(RUNCAM_REC action);

	HAL_StatusTypeDef Runcam_SimulateButton(RUNCAM_BUTTON button);

private:
	UART_HandleTypeDef* huart;

	uint8_t Runcam_CRC8(uint8_t *data, uint8_t len);

	HAL_StatusTypeDef Runcam_SendPacket(uint8_t command, uint8_t action);

};
