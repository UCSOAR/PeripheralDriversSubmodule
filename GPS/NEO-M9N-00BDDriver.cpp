
#include "NEO-M9N-00BDriver.hpp"
#include "SystemDefines.hpp"
#include <cstring>
#include <cstdlib>

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
	if (!valSetU1Type(static_cast<uint32_t>(CFG_MSGOUT_NMEA_ID_GGA_SPI), 1)) return false; //CFG_MSGOUT_NMEA_ID_GGA_SPI = 1 sets output rate to 1(testing for now)

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
	HAL_StatusTypeDef status = HAL_SPI_Transmit(_hspi, msg, sizeof(msg), 100);
	csHigh();

	if (status != HAL_OK) {
		SOAR_PRINT("GPS VALSET key=0x%08lX failed: HAL=%d spiState=%d spiErr=0x%08lX\n",
				   (unsigned long)keyId, (int)status, (int)_hspi->State, (unsigned long)_hspi->ErrorCode);
		return false;
	}

	// DEBUG: check that the receiver accepted this VALSET (prints ACK / NAK / timeout).
	// Only once the scheduler runs (e.g. "gps_init"): before that the tick is stopped and
	// a full print queue would block forever.
	if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
		bool acked = waitForAck(0x06, 0x8A, 1000);
		SOAR_PRINT("GPS VALSET key=0x%08lX val=%u -> %s\n",
				   (unsigned long)keyId, (unsigned int)value, acked ? "ACK" : "no ACK");
	}

	return true; // TODO: return acked once the receiver is confirmed to respond
}

bool NEOM9N00B::valSetU1Type(uint32_t keyId, uint8_t value){
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
		HAL_StatusTypeDef status = HAL_SPI_Transmit(_hspi, msg, sizeof(msg), 100);
		csHigh();

		if (status != HAL_OK) {
			SOAR_PRINT("GPS VALSET key=0x%08lX failed: HAL=%d spiState=%d spiErr=0x%08lX\n",
					   (unsigned long)keyId, (int)status, (int)_hspi->State, (unsigned long)_hspi->ErrorCode);
			return false;
		}
		return true;
		//return waitForAck(0x06, 0x8A, 1000);

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

bool NEOM9N00B::waitForAck(uint8_t expectedClass, uint8_t expectedId, uint32_t timeoutMs){
	constexpr uint16_t ACK_LEN = 10;        // B5 62 05 0x 02 00 cls id ckA ckB
	constexpr uint16_t CHUNK = 32;
	constexpr uint16_t MAX_READS = 500;     // HAL tick does not advance before the scheduler starts, so also bound by reads

	uint32_t start = HAL_GetTick();
	uint8_t txDummy[CHUNK];
	uint8_t window[CHUNK + ACK_LEN - 1];    // last ACK_LEN-1 bytes of the previous read + new chunk
	uint16_t carry = 0;

	uint32_t zeroBytes = 0, ffBytes = 0, otherBytes = 0;

	memset(txDummy, 0xFF, sizeof(txDummy));

	for (uint16_t reads = 0; reads < MAX_READS && (HAL_GetTick() - start) < timeoutMs; reads++) {
		csLow();
		HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(_hspi, txDummy, &window[carry], CHUNK, 100);
		csHigh();
		if (st != HAL_OK) {
			SOAR_PRINT("GPS ACK wait: SPI read failed HAL=%d\n", (int)st);
			return false;
		}

		for (uint16_t i = carry; i < carry + CHUNK; i++) {
			if (window[i] == 0x00) zeroBytes++;
			else if (window[i] == 0xFF) ffBytes++;
			else otherBytes++;
		}

		uint16_t len = carry + CHUNK;
		// Scan every position where a full ACK/NAK frame fits, including frames split across reads
		for (uint16_t i = 0; i + ACK_LEN <= len; i++) {
			// Look for UBX sync chars
			if (window[i] == 0xB5 && window[i + 1] == 0x62) {
				uint8_t msgClass = window[i + 2];
				uint8_t msgId    = window[i + 3];
				uint16_t length  = window[i + 4] | (window[i + 5] << 8);

				// ACK/NAK messages have class 0x05 and length 2
				if (msgClass == 0x05 && length == 2 &&
					window[i + 6] == expectedClass && window[i + 7] == expectedId) {

					// ACK-ACK
					if (msgId == 0x01) {
						return true;
					}

					// ACK-NAK
					if (msgId == 0x00) {
						SOAR_PRINT("GPS ACK wait: NAK for class=0x%02X id=0x%02X\n", expectedClass, expectedId);
						return false;
					}
				}
			}
		}

		// Keep the tail so a frame split across two reads is still found
		carry = ACK_LEN - 1;
		memmove(window, &window[len - carry], carry);
	}

	// Timed out: summarize what was on MISO (all 0x00 = nothing driving MISO, all 0xFF = receiver idle/no data)
	SOAR_PRINT("GPS ACK wait: timeout, MISO bytes 0x00=%lu 0xFF=%lu other=%lu\n",
			   (unsigned long)zeroBytes, (unsigned long)ffBytes, (unsigned long)otherBytes);
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

	static uint16_t idx = 0;
	static bool collecting = false;

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
				lineBuf[idx] = '\0';
				collecting = false;
				idx = 0;
				return true;
			}
		}
	}
	//return false if we do not have a full NMEA message yet
	return false;

}

bool NEOM9N00B::getGGALine(char* lineBuf, uint32_t timeoutMs){
	//collects the NMEALine until notified of terminator, gives up after timeoutMs
	uint32_t start = HAL_GetTick();
	while ((HAL_GetTick() - start) < timeoutMs) {
		if (collectNMEALine(lineBuf)) {
			if (strncmp(lineBuf, "$GPGGA", 6) == 0 ||
				strncmp(lineBuf, "$GNGGA", 6) == 0) {
				return true;
			}
		}
		else {
			osDelay(1); // yield so lower priority tasks (UART/Debug) are not starved
		}
	}
	return false;
}

void NEOM9N00B::ParseGpsData(GPSData* data)
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

