
#ifndef NEOM9N00B_HPP
#define NEOM9N00B_HPP


#define CFG_SPI_CPOL 0x10640002 //KeyID to Configure Clock Polarity
#define CFG_SPI_CPHASE 0x10640003 //KeyID Command to Configure Clock Phase
#define CFG_SPI_ENABLE 0x10640006 //KeyID to enable SPI

#define CFG_SPIOUTPROT_NMEA 0x107a0002 //KeyID to indicate if NMEA is output on SPI
#define CFG_SPIINPROT_NMEA 0x10790002 //KeyID to indicate if NMEA is input on SPI
#define CFG_SPIINPROT_UBX 0x10790001 //KeyID to indicate if UBX is input on SPI
#define CFG_SPIOUTPROT_UBX 0x107a0001 //KeyID to indicate if UBX is output on SPI

#define CFG_MSGOUT_NMEA_ID_GGA_SPI  0x209100be //KeyID to specify output rate of GGA on SPI

#define NMEA_MAX_LENGTH 82 //Max length for NMEA message including start and end characters

typedef struct
{
    int32_t    degrees_;
    int32_t    minutes_;
} LatLongType;

typedef struct
{
    int32_t     altitude_;
    char        unit_;
} AltitudeType;

typedef struct
{
    char            buffer_ [NMEA_MAX_LENGTH];
    uint32_t        time_;
    LatLongType     latitude_;
    LatLongType     longitude_;
    AltitudeType    antennaAltitude_;
    AltitudeType    geoidAltitude_;
    AltitudeType    totalAltitude_;
} GpsData;


enum class NEOM9N00B_Status{
	OK = 0

};

class NEOM9N00B{


public:

	NEOM9N00B();

	NEOM9N00B_Status Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* csPort, uint16_t csPin);

private:
	bool valSetLType(uint32_t keyID, uint8_t value)
	bool valSetU1Type(uint32_t keyID, uint8_t value)

	bool computeUBXChecksum(const uint8_t* data, uint16_t len, uint8_t& ckA, uint8_t& ckB);
	bool waitForAck(uint8_t expectedClass, uint8_t expectedId, uint32_t timeoutMs);
	bool collectNMEALine(char* lineBuf)
	bool readBytes(uint8_t* rxBuf, uint16_t len)
	bool getGGALine(char* lineBuf)

	void csHigh();
	void csLow();


	 SPI_HandleTypeDef* _hspi;
	 uint16_t _csPin;
	 GPIO_TypeDef* _csPort;


}
