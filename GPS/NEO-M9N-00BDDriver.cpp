
#include "NEO-M9N-00BDriver.hpp"

NEOM9N00B::NEOM9N00B(){

}

bool NEOM9N00B::Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* csPort, uint16_t csPin){
	_hspi = hspi;
	_csPort = csPort;
	_csPin = csPin;
	csHigh();
	if (!valSetLType(static_cast<uint32_t>(CFG_SPI_ENABLE), 1)) return false; // CFG-SPI-ENABLED = true
	if (!valSetLType(static_cast<uint32_t>(CFG_SPIINPROT_UBX), 1)) return false; // CFG-SPIINPROT-UBX = true set this just for config
	if (!valSetLType(static_cast<uint32_t>(CFG_SPIOUTPROT_UBX), 1)) return false; // CFG-SPIOUTPROT-UBX = true set this just for config
	if (!valSetLType(static_cast<uint32_t>(CFG_SPIOUTPROT_NMEA), 1)) return false; // CFG-SPIOUTPROT-NMEA = true
	if (!valSetLType(static_cast<uint32_t>(CFG_SPIINPROT_NMEA), 0)) return false; // CFG-SPIINPROT-NMEA = false, unless you need NMEA input
	if(!valSetU1Type(static_cast<uint32_t>(CFG_MSGOUT_NMEA_ID_GGA_SPI, 1)) return false; //CFG_MSGOUT_NMEA_ID_GGA_SPI = 1 sets output rate to 1(testing for now)
	return true;


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

	uint8_t ckA = 0, ckB = 0;

	// Checksum over CLASS, ID, LENGTH, PAYLOAD
	computeUBXChecksum(&msg[2], sizeof(msg) - 4, ckA, ckB); //message - 4 since we dont want first 2 bytes and last 2 bytes reserved for checksum

	//set checksum
	msg[sizeof(msg) - 2] = ckA;
	msg[sizeof(msg) - 1] = ckB;

	csLow();
	HAL_SPI_Transmit(_hspi, msg, sizeof(msg), HAL_MAX_DELAY);
	csHigh();

	return waitForAck(0x06, 0x8A, 1000);


}

bool NEOM9N00B::valSetU1Type(uint32 keyId, uint8_t value){
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

				value, //U1 value (unsigned 1 byte integer)

				0x00, 0x00		  //placeholders for checksum
		};

		uint8_t ckA = 0, ckB = 0;

		// Checksum over CLASS, ID, LENGTH, PAYLOAD
		computeUBXChecksum(&msg[2], sizeof(msg) - 4, ckA, ckB); //message - 4 since we dont want first 2 bytes and last 2 bytes reserved for checksum

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

bool NEOM9N00B::waitForAck(uint8_t expectedClass, uint8_t expectedId, uint32_t timeoutMs=1000){
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
	return false;

}

bool NEOM9N00B::readBytes(uint8_t* rxBuf, uint16_t len)
{
    uint8_t txDummy[64];

    if (len > sizeof(txDummy)) {
        return false;
    }

    memset(txDummy, 0xFF, len);

    csLow();
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(_hspi, txDummy, rxBuf, len, HAL_MAX_DELAY);
    csHigh();

    return (status == HAL_OK);
}

bool NEOM9N00B::collectNMEALine(char* lineBuf){

	uint8_t rx[32]; 	//make rx buffer to recieve bytes from SPI

	/*
	 * idx and collecting must be static because NMEA messages may not be completed in one SPI transaction.
	 * The function keeps track of whether the NMEA string is still being collected or not.
	 * Once the String is full which could be over multiple function calls, this function will return
	 * a true flag which can trigger a break in the loop calling this function. That true flag will be triggered
	 * when the end of the message has been reached.
	 */

	static uint16_t idx = 0
	static bool collecting = false

	//read the bytes from spi, return false if HAL status flag has error
	if(!readBytes(rx, sizeof(rx))){
		return false;
	}


	//iterate through bytes from rx buf to create message
	for(uint16_t i = 0; i < sizeof(rx); i++){
		char c = static_cast<char>(rx[i]);

		//if collecting is false, function must have either not been previously collecting or
		//the collection of a previous message has finished
		if(!collecting){
			// Therefore if collection is false we are looking for a new NMEA message to start
			// so we check if the char we are receiving is the start indicator or not
			if(c == '$'){
				idx = 0;
				collecting = true;
				lineBuf[idx++] = c;
			}
		}
		//If we are collecting, that means we clocked bytes from SPI that are the start or part of a message.
		//This means we can simply append the characters to the line buffer until we reach an end indicator
		//or a we reach NMEA message greater than the max length without a terminator (NMEA message is corrupt).
		else{
			if(idx < NMEA_MAX_LENGTH - 1){
				lineBuf[idx++] = c;
			}
			else{
				collecting = false;
				idx = 0;
				return false;
			}
		//Add null terminator if we hit an end indicator, indicates end of NMEA message; return true.
			if(c == '\n'){
				lineBuf[idx] = '\0'
				collecting = false;
				idx = 0;
				return true
			}
		}
	}
	//return false if we do not have a full NMEA message yet
	return false

}

bool NEOM9N00B::getGGALine(char* lineBuf){
	//collects the NMEALine until notified of terminator
	while (true) {
		if (collectNMEALine(lineBuf) {
			if (strncmp(lineBuf, "$GPGGA", 6) == 0 ||
				strncmp(lineBuf, "$GNGGA", 6) == 0) {
				return true;
			}
		}
	}
}

void NEOM9N00B::ParseGpsData(GpsData* data)
{

    char* gps_item = &data->buffer_[0];
    uint8_t item_len = 0;
    uint8_t counter = 0;
    uint8_t done = 0;
    char direction = 0;

    do
    {
        item_len = 0;
        //Iterate through the message, stop every time there is a comma
        for (uint8_t i = 0; i < NMEA_MAX_LENGTH + 1; i++) {
            if (gps_item[i] == ',') {
                gps_item[i] = 0;
                item_len = i;
                break;
            }
            //if hit null terminator break loop, exit function
            else if (gps_item[i] == 0) {
                item_len = i;
                done = 1;
                break;
            }
        }

        //skips gps item if it is null terminator
        if (gps_item[0] != 0) {
        	//based on the counts of the commas, you can determing the data being accessed
        	//this case switch based off the comma parses the data to its respective data type
            switch (counter)
            {
            case 1:
                data->time_ = (uint32_t)(atof(gps_item) * 100);
                break;

            case 2:
            {
                double latitude = atof(gps_item); // DDMM.MMMMMM
                data->latitude_.degrees_ = (int32_t)(latitude / 100);
                data->latitude_.minutes_ = (int32_t)((latitude - data->latitude_.degrees_ * 100) * 100000);
                break;
            }

            case 3:
                direction = *gps_item;
                if (direction == 'S') {
                    data->latitude_.degrees_ *= -1;
                    data->latitude_.minutes_ *= -1;
                }
                break;

            case 4:
            {
                double longitude = atof(gps_item); // DDDMM.MMMMMM
                data->longitude_.degrees_ = (int32_t)(longitude / 100);
                data->longitude_.minutes_ = (int32_t)((longitude - data->longitude_.degrees_ * 100) * 100000);
                break;
            }

            case 5:
                direction = *gps_item;
                if (direction == 'W') {
                    data->longitude_.degrees_ *= -1;
                    data->longitude_.minutes_ *= -1;
                }
                break;

            case 9:
                data->antennaAltitude_.altitude_ = (int32_t)(atof(gps_item) * 10);
                break;

            case 10:
                data->antennaAltitude_.unit_ = *gps_item;
                break;

            case 11:
                data->geoidAltitude_.altitude_ = (int32_t)(atof(gps_item) * 10);
                break;

            case 12:
                data->geoidAltitude_.unit_ = *gps_item;
                break;

            default:
                break;
            }
        }

        //increment the comma counter and set the gps_item pointer past the comma just read
        counter++;
        gps_item = &gps_item[item_len + 1];

    } while (done == 0);

    //set total altitude and antenna altitude

    data->totalAltitude_.altitude_ =
    		data->antennaAltitude_.altitude_ - data->geoidAltitude_.altitude_;
    data->totalAltitude_.unit_ = data->antennaAltitude_.unit_;

}

void NEOM9N00B::csHigh(){
	HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);
}

void NEOM9N00B::csLow(){
	HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_RESET);
}

