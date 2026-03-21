
#include "NEO-M9N-00BDriver.hpp"

NEOM9N00B::NEOM98N00B(){

}

NEOM9N00B::Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* csPort, uint16_t csPin){
	_hspi = hspi;
	_csPort = csPort;
	_csPin = csPin;
	csHigh();
	valSetLType(static_cast<uint32_t>(CFG_SPI_ENABLE), 1); // CFG-SPI-ENABLED = true
	valSetLType(static_cast<uint32_t>(CFG_SPIOUTPROT_NMEA), 1); // CFG-SPIOUTPROT-NMEA = true
	valSetLType(static_cast<uint32_t>(CFG_SPIINPROT_NMEA), 0); // CFG-SPIINPROT-NMEA = false, unless you need NMEA input
	valSetLType(static_cast<uint32_t>(CFG_SPIINPROT_UBX), 1); // CFG-SPIINPROT-UBX = true set this just for config
	valSetLType(static_cast<uint32_t>(CFG_SPIOUTPROT_UBX), 0); // CFG-SPIOUTPROT-UBX = true set this just for config


}

bool NEOM9N00B::valSetLType(uint32_t keyId, uint8_t value){

	uint8_t msg[] ={
		    0xB5, 0x62,       // sync
		    0x06, 0x8A,       // UBX-CFG-VALSET
		    0x09, 0x00,       // payload length = 9
		    0x00,             // version
		    0x01,             // layers: RAM
		    0x00, 0x00,       // reserved
			static_cast<uint8_t>( keyId        & 0xFF), //copy first byte
			static_cast<uint8_t>((keyId >> 8)  & 0xFF), //copy second byte
			static_cast<uint8_t>((keyId >> 16) & 0xFF), //copy third byte
			static_cast<uint8_t>((keyId >> 24) & 0xFF), // copy fourth byte

			value, //L value (either a 1 or a 0)

			0x00, 0x00		  //placeholders for checksum
	};

	uint8_t ckA, ckB = 0;

	// Checksum over CLASS, ID, LENGTH, PAYLOAD
	computeUBXChecksum(&msg[2], sizeof(msg) - 4, ckA, ckB) //message - 4 since we dont want first 2 bytes and last 2 bytes reserved for checksum

	//set checksum
	msg[sizeof(msg) - 2] = ckA;
	msg[sizeof(msg) - 1] = ckB;

	csLow();
	HAL_SPI_Transmit(_hspi, msg, sizeof(msg), HAL_MAX_DELAY);
	csHigh();

	return waitForAck(0x06, 0x8A, 1000);


}



void NEOM9N00B::computeUBXChecksum(const uint8_t* data, uint16_t len, uint8_t& ckA, uint8_t& ckB)
{
    ckA = 0;
    ckB = 0;

    for (uint16_t i = 0; i < len; i++) {
        ckA = ckA + data[i];
        ckB = ckB + ckA;
    }
}

void NEOM9N00B::waitForAck(uint8_t expectedClass, uint8_t expectedId, uint32_t timeoutMs=1000){
	uint32_t start = HAL_GetTick();
	uint8_t txDummy[32];
	uint8_t rxBuf[32];

	memset(txDummy, 0xFF, sizeof(txDummy));

	while ((HAL_GetTick() - start) < timeoutMs) {
		csLow();
		HAL_SPI_TransmitReceive(_hspi, txDummy, rxBuf, sizeof(rxBuf), 1000);
		csHigh();

		 for (uint8_t i = 0; i <= sizeof(rxBuf) - 10; i++) {
			// Look for UBX sync chars
			if (rxBuf[i] == 0xB5 && rxBuf[i + 1] == 0x62) {
				uint8_t msgClass = rxBuf[i + 2];
				uint8_t msgId    = rxBuf[i + 3];
				uint16_t length  = rxBuf[i + 4] | (rxBuf[i + 5] << 8);

				// ACK/NAK messages have class 0x05 and length 2
				if (msgClass == 0x05 && length == 2) {
					uint8_t ackedClass = rxBuf[i + 6];
					uint8_t ackedId    = rxBuf[i + 7];

					// ACK-ACK
					if (msgId == 0x01 &&
						ackedClass == expectedClass &&
						ackedId == expectedId) {
						return true;
					}

					// ACK-NAK
					if (msgId == 0x00 &&
						ackedClass == expectedClass &&
						ackedId == expectedId) {
						return false;
					}
				}
			}
		 }
	}

}

void NEOM9N00B::csHigh(){
	HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);
}

void NEOM9N00B::csLow(){
	HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_RESET);
}




