
#ifndef NEOM9N00B_HPP
#define NEOM9N00B_HPP


#define CFG_SPI_CPOL 0x10640002 //KeyID to Configure Clock Polarity
#define CFG_SPI_CPHASE 0x10640003 //KeyID Command to Configure Clock Phase
#define CFG_SPI_ENABLE 0x10640006 //KeyID to enable SPI

#define CFG_SPIOUTPROT_NMEA 0x107a0002 //KeyID to indicate if NMEA is output on SPI
#define CFG_SPIINPROT_NMEA 0x10790002 //KeyID to indicate if NMEA is input on SPI
#define CFG_SPIINPROT_UBX 0x10790001 //KeyID to indicate if UBX is input on SPI

enum class NEOM9N00B_Status{
	OK = 0

};

class NEOM9N00B{


public:

	NEOM9N00B();

	NEOM9N00B_Status Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* csPort, uint16_t csPin);

private:
	void valSetLType(uint32_t keyID, uint8_t value)

	void NEOM9N00B::computeUBXChecksum(const uint8_t* data, uint16_t len, uint8_t& ckA, uint8_t& ckB);

	void csHigh();
	void csLow();


	 SPI_HandleTypeDef* _hspi;
	 uint16_t _csPin;
	 GPIO_TypeDef* _csPort;


}
